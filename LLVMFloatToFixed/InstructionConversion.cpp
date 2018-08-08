#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <cmath>
#include <cassert>
#include "LLVMFloatToFixedPass.h"

using namespace llvm;
using namespace flttofix;

#define defaultFixpType @SYNTAX_ERROR@


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertInstruction(Module& m, Instruction *val, FixedPointType& fixpt)
{
  Value *res = Unsupported;
  
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    res = convertAlloca(alloca, fixpt);
  } else if (LoadInst *load = dyn_cast<LoadInst>(val)) {
    res = convertLoad(load, fixpt);
  } else if (StoreInst *store = dyn_cast<StoreInst>(val)) {
    res = convertStore(store);
  } else if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(val)) {
    res = convertGep(gep, fixpt);
  } else if (PHINode *phi = dyn_cast<PHINode>(val)) {
    res = convertPhi(phi, fixpt);
  } else if (SelectInst *select = dyn_cast<SelectInst>(val)) {
    res = convertSelect(select, fixpt);
  } else if (CallInst *call = dyn_cast<CallInst>(val)) { //TODO Handle InvokeInst
    res = convertCall(call, fixpt);
  } else if (ReturnInst *ret = dyn_cast<ReturnInst>(val)) {
    res = convertRet(ret, fixpt);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) { //llvm/include/llvm/IR/Instruction.def for more info
    if (instr->isBinaryOp()) {
      res = convertBinOp(instr, fixpt);
    } else if (CastInst *cast = dyn_cast<CastInst>(instr)){
      res = convertCast(cast, fixpt);
    } else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(val)) {
      res = convertCmp(fcmp);
    }
  }

  if (res==Unsupported) {
    res = fallback(dyn_cast<Instruction>(val), fixpt);
  }
  
  if (res && res != Unsupported && !(res->getType()->isVoidTy()) && isFloatType(val->getType())) {
    std::string tmpstore;
    raw_string_ostream tmp(tmpstore);
    if (res->hasName())
      tmp << res->getName().str() << ".";
    else if (val->hasName())
      tmp << val->getName().str() << ".";
    tmp << fixPType(val);
    res->setName(tmp.str());
  }

  return res ? res : ConversionError;
}


Value *FloatToFixed::convertAlloca(AllocaInst *alloca, const FixedPointType& fixpt)
{
  Type *prevt = alloca->getAllocatedType();
  Type *newt = getLLVMFixedPointTypeForFloatType(prevt, fixpt);
  if (newt == prevt)
    return alloca;

  Value *as = alloca->getArraySize();
  AllocaInst *newinst = new AllocaInst(newt, alloca->getType()->getPointerAddressSpace(), as, alloca->getAlignment());
  
  newinst->setUsedWithInAlloca(alloca->isUsedWithInAlloca());
  newinst->setSwiftError(alloca->isSwiftError());
  newinst->insertAfter(alloca);
  return newinst;
}


Value *FloatToFixed::convertLoad(LoadInst *load, FixedPointType& fixpt)
{
  Value *ptr = load->getPointerOperand();
  Value *newptr = operandPool[ptr];
  if (newptr == ConversionError)
    return nullptr;
  assert(newptr && "a load can't be in the conversion queue just because");
  fixpt = fixPType(newptr);

  LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
    load->getAlignment(), load->getOrdering(), load->getSyncScopeID());
  newinst->insertAfter(load);
  return newinst;
}


Value *FloatToFixed::convertStore(StoreInst *store)
{
  Value *ptr = store->getPointerOperand();
  Value *val = store->getValueOperand();
  
  Value *newptr = matchOp(ptr);
  if (!newptr)
    return nullptr;
  
  Value *newval;
  if (val->getType()->isFloatingPointTy()) {
    FixedPointType valtype;
    Type *peltype = newptr->getType()->getPointerElementType();
    
    if (peltype->isFloatingPointTy()) {
      newval = translateOrMatchOperand(val, valtype, store);
      newval = genConvertFixToFloat(newval, valtype, peltype);
      
    } else if (peltype->isIntegerTy()) {
      valtype = fixPType(newptr);
      newval = translateOrMatchOperandAndType(val, valtype, store);
      
    } else
      return nullptr;
  } else {
    newval = matchOp(val);
  }
  if (!newval)
    return nullptr;
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSyncScopeID());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertGep(GetElementPtrInst *gep, FixedPointType& fixpt)
{
  if (info[gep].isRoot && info[gep].isBacktrackingNode) {
    dbgs() << "*** UGLY HACK *** ";
    /* till we can flag a structure for conversion we bitcast away the
     * item pointer to a fixed point type and hope everything still works */
    BitCastInst *bci = new BitCastInst(gep, getLLVMFixedPointTypeForFloatType(gep->getType(), fixpt));
    bci->setName(gep->getName() + ".haxfixp");
    bci->insertAfter(gep);
    bci->print(dbgs());
    dbgs() << "\n";
    return bci;
  }
  IRBuilder <> builder (gep);
  Value *newval = matchOp(gep->getPointerOperand());
  if (!newval)
    return nullptr;
  fixpt = fixPType(newval);

  std::vector<Value*> vals;
  for (auto a : gep->operand_values()) {
    vals.push_back(a);
  }
  vals.erase(vals.begin());

  ArrayRef <Value*> idxlist(vals);
  return builder.CreateInBoundsGEP(newval,idxlist);
}


Value *FloatToFixed::convertPhi(PHINode *load, FixedPointType& fixpt)
{
  if (!load->getType()->isFloatingPointTy()) {
    /* in the conversion chain the floating point number was converted to
     * an int at some point; we just upgrade the incoming values in place */

    /* if all of our incoming values were not converted, we want to propagate
     * that information across the phi. If at least one of them was converted
     * the phi is converted as well; otherwise it is not. */
    bool donesomething = false;
    
    if (isFloatType(load->getType())) {
      dbgs() << "warning: pointer phi + multiple fixed point types are tricky";
    }

    for (int i=0; i<load->getNumIncomingValues(); i++) {
      Value *thisval = load->getIncomingValue(i);
      Value *newval = operandPool[thisval];
      if (newval && newval != ConversionError) {
        load->setIncomingValue(i, newval);
        donesomething = true;
      }
    }
    return donesomething ? load : nullptr;
  }

  /* if we have to do a type change, create a new phi node. The new type is for
   * sure that of a fixed point value; because the original type was a float
   * and thus all of its incoming values were floats */
  PHINode *newphi = PHINode::Create(fixpt.toLLVMType(load->getContext()),
    load->getNumIncomingValues());

  for (int i=0; i<load->getNumIncomingValues(); i++) {
    Value *thisval = load->getIncomingValue(i);
    BasicBlock *thisbb = load->getIncomingBlock(i);
    Value *newval = translateOrMatchOperandAndType(thisval, fixpt, load);
    if (!newval) {
      delete newphi;
      return nullptr;
    }
    newphi->addIncoming(newval, thisbb);
  }
  newphi->insertAfter(load);
  return newphi;
}


Value *FloatToFixed::convertSelect(SelectInst *sel, FixedPointType& fixpt)
{
  /* the condition is always a bool (i1) or a vector of bools */
  Value *newcond = matchOp(sel->getCondition());
  
  if (!sel->getType()->isFloatingPointTy()) {
    if (isFloatType(sel->getType())) {
      dbgs() << "warning: select + multiple fixed point types are tricky";
    }
    Value *newtruev = matchOp(sel->getTrueValue());
    Value *newfalsev = matchOp(sel->getFalseValue());

    /* like phi, upgrade in place */
    if (newcond)
      sel->setCondition(newcond);
    if (newtruev)
      sel->setTrueValue(newtruev);
    if (newfalsev)
      sel->setFalseValue(newfalsev);
    return sel;
  }

  /* otherwise create a new one */
  Value *newtruev = translateOrMatchOperandAndType(sel->getTrueValue(), fixpt, sel);
  Value *newfalsev = translateOrMatchOperandAndType(sel->getFalseValue(), fixpt, sel);
  if (!newtruev || !newfalsev || !newcond)
    return nullptr;

  SelectInst *newsel = SelectInst::Create(newcond, newtruev, newfalsev);
  newsel->insertAfter(sel);
  return newsel;
}


Value *FloatToFixed::convertCall(CallInst *call, FixedPointType& fixpt)
{
  /* If the function return a float the new return type will be a fix point of type fixpt,
   * otherwise the return type is left unchanged.*/
  IRBuilder<> builder(call->getNextNode());
  Function *oldF = call->getCalledFunction();

  if(oldF->getName() == "printf" || oldF->getName().startswith("llvm.") || oldF->getName() == "logf" ||
      oldF->getName() == "expf" || oldF->getName() == "sqrtf") //TODO blacklisted function
    return Unsupported;

  std::vector<Value*> convArgs;
  std::vector<Type*> typeArgs;
  std::vector<std::pair<int, FixedPointType>> fixArgs; //for match right function

  if(oldF->getReturnType()->isFloatingPointTy())
    fixArgs.push_back(std::pair<int, FixedPointType>(-1, fixpt)); //ret value in signature

  int i=0;
  for (auto *it = call->arg_begin(); it != call->arg_end(); it++,i++) {
    if (hasInfo(*it)) {
      convArgs.push_back(translateOrMatchOperand(*it, info[*it].fixpType, call));
      fixArgs.push_back(std::pair<int, FixedPointType>(i,info[*it].fixpType));
    } else {
      convArgs.push_back(dyn_cast<Value>(it));
    }
    typeArgs.push_back(convArgs[i]->getType());
  }

  Function *newF;
  for (FunInfo f : functionPool[oldF]) { //get right function
    if (f.fixArgs == fixArgs) {
      newF = f.newFun;
      DEBUG(dbgs() << *call <<  " use converted function : " <<
                   newF->getName() << " " << *newF->getType() << "\n";);
      return builder.CreateCall(newF, convArgs);
    }
  }

  assert("Every function should be cloned previously!\n");
  
  FunctionType *newFunTy = FunctionType::get(
      oldF->getReturnType()->isFloatingPointTy() ?
      fixpt.toLLVMType(call->getContext()) :
      oldF->getReturnType(),
      typeArgs, oldF->isVarArg());
  
  
  newF = Function::Create(newFunTy, oldF->getLinkage(), oldF->getName() + "_fixp", oldF->getParent());

  FunInfo funInfo; //add to pool
  funInfo.newFun = newF;
  funInfo.fixArgs = fixArgs;
  functionPool[oldF].push_back(funInfo);

  convertFun(oldF, newF, convArgs, fixpt);
  return builder.CreateCall(newF, convArgs);
}


Value *FloatToFixed::convertRet(ReturnInst *ret, FixedPointType& fixpt)
{
  Value *v;
  if (ret->getReturnValue()->getType()->isIntegerTy()) {
    //if return an int we shouldn't return a fix point, go into fallback
    return Unsupported;
  }
  if (Function* f = dyn_cast<Function>(ret->getParent()->getParent())) {
    if (!f->getReturnType()->isIntegerTy()) {
      //the function doesn't return a fixp, don't convert the ret
      Value *oldval = ret->getReturnValue();
      Value *newval = operandPool[oldval];
      v = oldval->getType()->isFloatingPointTy()
          ? dyn_cast<Instruction>(genConvertFixToFloat(newval, fixPType(newval), oldval->getType()))
          : dyn_cast<Instruction>(newval);
      
      ret->setOperand(0,v);
      return ret;
    }
  }

  v = translateOrMatchOperand(ret->getOperand(0), fixpt);
  ret->setOperand(0,v);
}


Value *FloatToFixed::convertBinOp(Instruction *instr, const FixedPointType& fixpt)
{
  /*le istruzioni Instruction::
    [Add,Sub,Mul,SDiv,UDiv,SRem,URem,Shl,LShr,AShr,And,Or,Xor]
    vengono gestite dalla fallback e non in questa funzione */
  if (!instr->getType()->isFloatingPointTy())
    return Unsupported;
  
  IRBuilder<> builder(instr->getNextNode());
  int opc = instr->getOpcode();

  FixedPointType intype1 = fixpt, intype2 = fixpt;
  Value *val1 = translateOrMatchOperand(instr->getOperand(0), intype1, instr);
  Value *val2 = translateOrMatchOperand(instr->getOperand(1), intype2, instr);
  if (!val1 || !val2)
    return nullptr;

  if (opc == Instruction::FAdd || opc == Instruction::FSub || opc == Instruction::FRem) {
    val1 = genConvertFixedToFixed(val1, intype1, fixpt);
    val2 = genConvertFixedToFixed(val2, intype2, fixpt);
    
    if (opc == Instruction::FAdd)
      return builder.CreateBinOp(Instruction::Add, val1, val2);
    
    else if (opc == Instruction::FSub)
      return builder.CreateBinOp(Instruction::Sub, val1, val2);
    
    else if (opc == Instruction::FRem) {
      if (fixpt.isSigned)
        return builder.CreateBinOp(Instruction::SRem, val1, val2);
      else
        return builder.CreateBinOp(Instruction::URem, val1, val2);
    }

  } else if (opc == Instruction::FMul || opc == Instruction::FDiv) {
    FixedPointType intermtype(
      fixpt.isSigned,
      intype1.fracBitsAmt + intype2.fracBitsAmt,
      intype1.bitsAmt + intype2.bitsAmt);
    Type *dbfxt = intermtype.toLLVMType(instr->getContext());
  
    if (opc == Instruction::FMul) {
      Value *ext1 = intype1.isSigned ? builder.CreateSExt(val1, dbfxt) : builder.CreateZExt(val1, dbfxt);
      Value *ext2 = intype2.isSigned ? builder.CreateSExt(val2, dbfxt) : builder.CreateZExt(val2, dbfxt);
      Value *fixop = builder.CreateMul(ext1, ext2);
      return genConvertFixedToFixed(fixop, intermtype, fixpt);
      
    } else {
      FixedPointType fixoptype(
        fixpt.isSigned,
        intype1.fracBitsAmt,
        intype1.bitsAmt + intype2.bitsAmt);
      Value *ext1 = genConvertFixedToFixed(val1, intype1, intermtype, instr);
      Value *ext2 = intype1.isSigned ? builder.CreateSExt(val2, dbfxt) : builder.CreateZExt(val2, dbfxt);
      Value *fixop = fixpt.isSigned ? builder.CreateSDiv(ext1, ext2) : builder.CreateUDiv(ext1, ext2);
      return genConvertFixedToFixed(fixop, fixoptype, fixpt);
    }
  }
  
  return Unsupported;
}


Value *FloatToFixed::convertCmp(FCmpInst *fcmp)
{
  Value *op1 = fcmp->getOperand(0);
  Value *op2 = fcmp->getOperand(1);
  
  FixedPointType cmptype;
  FixedPointType t1, t2;
  bool hasinfo1 = hasInfo(op1), hasinfo2 = hasInfo(op2);
  if (hasinfo1 && hasinfo2) {
    t1 = fixPType(op1);
    t2 = fixPType(op2);
  } else if (hasinfo1) {
    t1 = fixPType(op1);
    t2 = t1;
    t2.isSigned = true;
  } else if (hasinfo2) {
    t2 = fixPType(op2);
    t1 = t2;
    t1.isSigned = true;
  }
  bool mixedsign = t1.isSigned != t2.isSigned;
  int intpart1 = t1.bitsAmt - t1.fracBitsAmt + (mixedsign ? t1.isSigned : 0);
  int intpart2 = t2.bitsAmt - t2.fracBitsAmt + (mixedsign ? t2.isSigned : 0);
  cmptype.isSigned = t1.isSigned || t2.isSigned;
  cmptype.fracBitsAmt = std::max(t1.fracBitsAmt, t2.fracBitsAmt);
  cmptype.bitsAmt = std::max(intpart1, intpart2) + cmptype.fracBitsAmt;
  
  Value *val1 = translateOrMatchOperandAndType(op1, cmptype, fcmp);
  Value *val2 = translateOrMatchOperandAndType(op2, cmptype, fcmp);
  
  IRBuilder<> builder(fcmp->getNextNode());
  CmpInst::Predicate ty;
  int pr = fcmp->getPredicate();
  bool swapped = false;

  //se unordered swappo, converto con la int, e poi mi ricordo di riswappare
  if (!CmpInst::isOrdered(fcmp->getPredicate())) {
    pr = fcmp->getInversePredicate();
    swapped = true;
  }

  if (pr == CmpInst::FCMP_OEQ){
    ty = CmpInst::ICMP_EQ;
  } else if (pr == CmpInst::FCMP_ONE) {
    ty = CmpInst::ICMP_NE;
  } else if (pr == CmpInst::FCMP_OGT) {
    ty = cmptype.isSigned ? CmpInst::ICMP_SGT : CmpInst::ICMP_UGT;
  } else if (pr == CmpInst::FCMP_OGE) {
    ty = cmptype.isSigned ? CmpInst::ICMP_SGE : CmpInst::ICMP_UGE;
  } else if (pr == CmpInst::FCMP_OLE) {
    ty = cmptype.isSigned ? CmpInst::ICMP_SLE : CmpInst::ICMP_ULE;
  } else if (pr == CmpInst::FCMP_OLT) {
    ty = cmptype.isSigned ? CmpInst::ICMP_SLT : CmpInst::ICMP_ULT;
  } else if (pr == CmpInst::FCMP_ORD) {
    ;
    //TODO gestione NaN
  } else if (pr == CmpInst::FCMP_TRUE) {
    /* there is no integer-only always-true / always-false comparison
     * operator... so we roll out our own by producing a tautology */
    return builder.CreateICmpEQ(
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0));
  } else if (pr == CmpInst::FCMP_FALSE) {
    return builder.CreateICmpNE(
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0));
  }

  if (swapped) {
    ty = CmpInst::getInversePredicate(ty);
  }

  return val1 && val2
    ? builder.CreateICmp(ty, val1, val2)
    : nullptr;
}


Value *FloatToFixed::convertCast(CastInst *cast, const FixedPointType& fixpt)
{
  /* le istruzioni Instruction::
   * - [FPToSI,FPToUI,SIToFP,UIToFP] vengono gestite qui
   * - [Trunc,ZExt,SExt] vengono gestite dalla fallback e non qui
   * - [PtrToInt,IntToPtr,BitCast,AddrSpaceCast] potrebbero portare errori */

  IRBuilder<> builder(cast->getNextNode());
  Value *operand = cast->getOperand(0);
  
  if (operand->getType()->isFloatingPointTy()) {
    /* fptosi, fptoui, fptrunc, fpext */
    if (cast->getOpcode() == Instruction::FPToSI) {
      return translateOrMatchOperandAndType(operand, FixedPointType(cast->getType(), true), cast);

    } else if (cast->getOpcode() == Instruction::FPToUI) {
      return translateOrMatchOperandAndType(operand, FixedPointType(cast->getType(), false), cast);

    } else if (cast->getOpcode() == Instruction::FPTrunc ||
               cast->getOpcode() == Instruction::FPExt) {
      return translateOrMatchOperandAndType(operand, fixpt, cast);
    }
  } else {
    /* sitofp, uitofp */
    Value *val = matchOp(operand);
    
    if (cast->getOpcode() == Instruction::SIToFP) {
      return genConvertFixedToFixed(val, FixedPointType(val->getType(), true), fixpt);

    } else if (cast->getOpcode() == Instruction::UIToFP) {
      return genConvertFixedToFixed(val, FixedPointType(val->getType(), false), fixpt);
    }
  }

  return Unsupported;
}


Value *FloatToFixed::fallback(Instruction *unsupp, FixedPointType& fixpt)
{
  Value *fallval;
  Instruction *fixval;
  std::vector<Value *> newops;

  DEBUG(dbgs() << "[Fallback] attempt to wrap not supported operation:\n" << *unsupp << "\n");
  FallbackCount++;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);

    Value *cvtfallval = operandPool[fallval];
    if (cvtfallval == ConversionError || (cvtfallval && cvtfallval->getType()->isPointerTy())) {
      DEBUG(dbgs() << "  bail out on missing operand " << i+1 << " of " << n << "\n");
      return nullptr;
    }

    //se è stato precedentemente sostituito e non è un puntatore
    if (cvtfallval) {
      /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
        il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
        Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
      fixval = fallval->getType()->isFloatingPointTy()
        ? dyn_cast<Instruction>(genConvertFixToFloat(cvtfallval, fixPType(cvtfallval), fallval->getType()))
        : dyn_cast<Instruction>(cvtfallval);

      DEBUG(dbgs() << "  Substituted operand number : " << i+1 << " of " << n << "\n");
      newops.push_back(fixval);
    } else {
      newops.push_back(fallval);
    }
  }

  for (int i=0, n=unsupp->getNumOperands(); i<n; i++) {
    unsupp->setOperand(i, newops[i]);
  }
  DEBUG(dbgs() << "  mutated operands to:\n" << *unsupp << "\n");
  if (unsupp->getType()->isFloatingPointTy()) {
    Value *fallbackv = genConvertFloatToFix(unsupp, fixpt, unsupp);
    if (unsupp->hasName())
      fallbackv->setName(unsupp->getName() + ".fallback");
    return fallbackv;
  }
  return unsupp;
}


void FloatToFixed::convertFun(Function *oldF, Function *newF, std::vector<Value*> convArgs, FixedPointType& retType)
{
  ValueToValueMapTy mapArgs;
  std::vector<Value*> roots;

  Function::arg_iterator newIt = newF->arg_begin();
  Function::arg_iterator oldIt = oldF->arg_begin();
  for (; oldIt != oldF->arg_end() ; oldIt++, newIt++) {
    newIt->setName(oldIt->getName());
    mapArgs.insert(std::make_pair(oldIt, newIt));
  }

  SmallVector<ReturnInst*,100> returns;
  CloneFunctionInto(newF, oldF, mapArgs, true, returns);


  oldIt = oldF->arg_begin();
  newIt = newF->arg_begin();
  for (int i=0; oldIt != oldF->arg_end() ; oldIt++, newIt++,i++) {
    if (oldIt->getType() != newIt->getType()){
      // Mark the alloca used for the argument in the O0 optimization level
      info[newIt->user_begin()->getOperand(1)] = info[convArgs[i]];
      roots.push_back(newIt->user_begin()->getOperand(1));

      /*dbgs() << " +++++++++++++++++++++++++++ " << *newIt << "\n"; //Try to handle different OX lvl opt
      operandPool[dyn_cast<Value>(newIt)] = dyn_cast<Value>(newIt);
      info[dyn_cast<Value>(newIt)] = info[convArgs[i]];
      roots.push_back(newIt);
      for (auto val = newIt->user_begin(); val != newIt->user_end(); val++) {
        dbgs() << " ######################################## " << **val << "\n";

        info[*val] = info[convArgs[i]];
        roots.push_back(*val);
      }
      info[dyn_cast<Value>(newIt)] = info[convArgs[i]];
      operandPool[dyn_cast<Value>(newIt)] = dyn_cast<Value>(newIt);*/

      //append fixp info to arg name
      std::string tmpstore;
      raw_string_ostream tmp(tmpstore);
      tmp << fixPType(newIt->user_begin()->getOperand(1));
      newIt->setName(newIt->getName() + "." + tmp.str());
    }
  }

  // If return a float, all the return inst should have a fix point (independently from the conv queue)
  if (oldF->getReturnType()->isFloatingPointTy()) {
    for (ReturnInst *v : returns) {
      roots.push_back(v);
      info[v].fixpType = retType;
      info[v].fixpTypeRootDistance = 0;
    }
  }


  for (Value *v : roots) {
    dbgs() << "Roots : ";
    v->print(dbgs());
    dbgs() << "\n-------\n";
  }

  std::vector<Value*> vals;
  buildConversionQueueForRootValues(roots, vals);

  DEBUG(dbgs() << "Converting function : " << oldF->getName() << " = ";
    oldF->getType()->print(dbgs());
    dbgs() << " --> " << newF->getName() << " = ";
    newF->getType()->print(dbgs());
    dbgs() << "\n";
    printConversionQueue(vals););

  performConversion(*newF->getParent(), vals);
  cleanup(vals);

  dbgs() << "Converted function : \n";
  newF->print(dbgs());
  FunctionCreated++;
}