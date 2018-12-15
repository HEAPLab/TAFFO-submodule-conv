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
  } else if (isa<CallInst>(val) || isa<InvokeInst>(val)) {
    CallSite *call = new CallSite(val);
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
  AllocaInst *newinst = new AllocaInst(newt, as, alloca->getAlignment());
  
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
    load->getAlignment(), load->getOrdering(), load->getSynchScope());
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
  if (isFloatType(val->getType())) {
    FixedPointType valtype;
    Type *peltype = newptr->getType()->getPointerElementType();
    
    if (peltype->isFloatingPointTy()) {
      newval = translateOrMatchOperand(val, valtype, store);
      newval = genConvertFixToFloat(newval, valtype, peltype);
  
    } else if (peltype->isIntegerTy()) {
      valtype = fixPType(newptr);
      newval = translateOrMatchOperandAndType(val, valtype, store);
      
    } else if (peltype->isPointerTy()){
      newval = translateOrMatchOperand(val, valtype, store);
      if (!newval || newval->getType() != peltype) {
        BitCastInst *bc = new BitCastInst(!newval ? val : newval,peltype);
        cpMetaData(bc,!newval ? val : newval);
        bc->insertBefore(store);
        newval = bc;
      }
      DEBUG(dbgs()<< "[Store] Detect pointer store :"<< *store << "\n");
      
    } else {
      return nullptr;
    }
    
  } else if (isa<Argument>(val) && hasInfo(val)) {
    /* converted function parameter in a cloned function */
    if (val->getType()->isIntegerTy()) {
      newval = genConvertFixedToFixed(val, fixPType(val), fixPType(newptr), store);
    } else {
      /* pointers */
      newval = matchOp(val);
      assert(fixPType(newval) == fixPType(newptr) && "mismatching types on argument");
    }
    
  } else {
    newval = matchOp(val);

  }
  if (!newval)
    return nullptr;
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSynchScope());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertGep(GetElementPtrInst *gep, FixedPointType& fixpt)
{
  if (valueInfo(gep)->isRoot && valueInfo(gep)->isBacktrackingNode) {
    dbgs() << "*** UGLY HACK *** ";
    /* till we can flag a structure for conversion we bitcast away the
     * item pointer to a fixed point type and hope everything still works */
    BitCastInst *bci = new BitCastInst(gep, getLLVMFixedPointTypeForFloatType(gep->getType(), fixpt));
    cpMetaData(bci,gep);
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
    Value *newval = translateOrMatchOperandAndType(thisval, fixpt, thisbb->getFirstNonPHI());
    if (!newval) {
      delete newphi;
      return nullptr;
    }
    Instruction *inst2 = dyn_cast<Instruction>(newval);
    if (inst2) {
      DEBUG(dbgs() << "warning: new phi value " << *inst2 << " not coming from the obviously correct BB\n");
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


Value *FloatToFixed::convertCall(CallSite *call, FixedPointType& fixpt)
{
  /* If the function return a float the new return type will be a fix point of type fixpt,
   * otherwise the return type is left unchanged.*/
  Function *oldF = call->getCalledFunction();

  if(isSpecialFunction(oldF))
    return Unsupported;

  std::vector<Value*> convArgs;
  std::vector<Type*> typeArgs;
  std::vector<std::pair<int, FixedPointType>> fixArgs; //for match right function
  
  if(isFloatType(oldF->getReturnType()))
    fixArgs.push_back(std::pair<int, FixedPointType>(-1, fixpt)); //ret value in signature

  int i=0;
  for (auto *it = call->arg_begin(); it != call->arg_end(); it++,i++) {
    if (hasInfo(*it)) {
      convArgs.push_back(translateOrMatchOperand(*it, fixPType(*it), call->getInstruction()));
      fixArgs.push_back(std::pair<int, FixedPointType>(i, fixPType(*it)));
    } else {
      convArgs.push_back(dyn_cast<Value>(it));
    }
    typeArgs.push_back(convArgs[i]->getType());
  }

  Function *newF;
  for (FunInfo f : functionPool[oldF]) { //get right function
    if (f.fixArgs == fixArgs) {
      newF = f.newFun;
      DEBUG(dbgs() << *(call->getInstruction()) <<  " use converted function : " <<
                   newF->getName() << " " << *newF->getType() << "\n";);
      
      if (call->isCall()) {
        CallInst *newCall = CallInst::Create(newF, convArgs);
        newCall->setCallingConv(call->getCallingConv());
        newCall->insertBefore(call->getInstruction());
        return newCall;
      } else if (call->isInvoke()) {
        InvokeInst *invk = dyn_cast<InvokeInst>(call->getInstruction());
        InvokeInst *newInvk = InvokeInst::Create(newF, invk->getNormalDest(), invk->getUnwindDest(), convArgs);
        newInvk->setCallingConv(call->getCallingConv());
        newInvk->insertBefore(invk);
        return newInvk;
      } else {
        assert("Unknown CallSite type");
      }
    }
  }
  
  dbgs() << "[Error]" << *(call->getInstruction()) << " doesn't find a function to call!\n";
  dbgs() << "new types should have been ";
  for (auto pair: fixArgs) {
    dbgs() << "(" << pair.first << ", " << pair.second << ") ";
  }
  dbgs() << "\n";
  assert(!newF && "Every function should have been already cloned!");
  return Unsupported;
}


Value *FloatToFixed::convertRet(ReturnInst *ret, FixedPointType& fixpt)
{
  Value *v;
  Value *oldv = ret->getReturnValue();
  Instruction *newRet;
  
  if (oldv->getType()->isIntegerTy()) {
    //if return an int we shouldn't return a fix point, go into fallback
    return Unsupported;
  }
  if (Function* f = dyn_cast<Function>(ret->getParent()->getParent())) {
    if (!f->getReturnType()->isIntegerTy()) {
      //the function doesn't return a fixp, don't convert the ret
      Value *newval = operandPool[oldv];
      v = oldv->getType()->isFloatingPointTy()
          ? dyn_cast<Instruction>(genConvertFixToFloat(newval, fixPType(newval), oldv->getType()))
          : dyn_cast<Instruction>(newval);
      
      ret->setOperand(0,v);
      newRet = ret->clone();
      newRet->insertAfter(ret);
      return newRet;
    }
  }

  /* force the value returned to be of the correct type */
  v = translateOrMatchOperandAndType(ret->getOperand(0), fixpt);
  
  ret->setOperand(0,v);
  newRet = ret->clone();
  newRet->insertAfter(ret);
  return newRet;
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
      cpMetaData(ext1,val1);
      cpMetaData(ext2,val2);
      cpMetaData(fixop,ext1);
      cpMetaData(fixop,ext2);
      updateFPTypeMetadata(fixop, intermtype.isSigned, intype1.fracBitsAmt, intermtype.bitsAmt);
      return genConvertFixedToFixed(fixop, intermtype, fixpt);
      
    } else {
      FixedPointType fixoptype(
        fixpt.isSigned,
        intype1.fracBitsAmt,
        intype1.bitsAmt + intype2.bitsAmt);
      Value *ext1 = genConvertFixedToFixed(val1, intype1, intermtype, instr);
      Value *ext2 = intype1.isSigned ? builder.CreateSExt(val2, dbfxt) : builder.CreateZExt(val2, dbfxt);
      Value *fixop = fixpt.isSigned ? builder.CreateSDiv(ext1, ext2) : builder.CreateUDiv(ext1, ext2);
      cpMetaData(ext2,val2);
      cpMetaData(fixop,ext2);
      updateFPTypeMetadata(fixop, fixoptype.isSigned, fixoptype.fracBitsAmt, fixoptype.bitsAmt);
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
  
  if (BitCastInst *bc = dyn_cast<BitCastInst>(cast)) {
    Value *newOperand = operandPool[operand];
    if (newOperand && newOperand!=ConversionError){
      return builder.CreateBitCast(
          newOperand,
          bc->getDestTy());
    } else {
      return Unsupported;
    }
  }
  
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
    if (cvtfallval == ConversionError) {
      DEBUG(dbgs() << "  bail out on missing operand " << i+1 << " of " << n << "\n");
      return nullptr;
    }

    //se è stato precedentemente sostituito e non è un puntatore
    if (cvtfallval) {
      /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
        il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
        Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
      if (cvtfallval->getType()->isPointerTy()) {
        BitCastInst *bc = new BitCastInst(cvtfallval,unsupp->getOperand(i)->getType());
        cpMetaData(bc,cvtfallval);
        bc->insertBefore(unsupp);
        fixval = bc;
      } else {
        fixval = fallval->getType()->isFloatingPointTy()
                 ? dyn_cast<Instruction>(genConvertFixToFloat(cvtfallval, fixPType(cvtfallval), fallval->getType()))
                 : dyn_cast<Instruction>(cvtfallval);
      }
      DEBUG(dbgs() << "  Substituted operand number : " << i+1 << " of " << n << "\n");
      newops.push_back(fixval);
    } else {
      newops.push_back(fallval);
    }
  }
  
  Instruction *tmp = unsupp->clone();
  if (!tmp->getType()->isVoidTy())
    tmp->setName(unsupp->getName() + ".flt");
  tmp->insertAfter(unsupp);
  
  for (int i=0, n=tmp->getNumOperands(); i<n; i++) {
    tmp->setOperand(i, newops[i]);
  }
  DEBUG(dbgs() << "  mutated operands to:\n" << *tmp << "\n");
  if (tmp->getType()->isFloatingPointTy()) {
    Value *fallbackv = genConvertFloatToFix(tmp, fixpt, tmp);
    if (tmp->hasName())
      fallbackv->setName(tmp->getName() + ".fallback");
    return fallbackv;
  }
  return tmp;
}

