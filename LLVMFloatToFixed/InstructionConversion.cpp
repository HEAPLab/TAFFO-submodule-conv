#include <cmath>
#include <cassert>
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
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"

using namespace llvm;
using namespace flttofix;
using namespace taffo;

#define defaultFixpType @SYNTAX_ERROR@


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertInstruction(Module& m, Instruction *val, TypeOverlay *&fixpt)
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
  } else if (ExtractValueInst *ev = dyn_cast<ExtractValueInst>(val)) {
    res = convertExtractValue(ev, fixpt);
  } else if (InsertValueInst *iv = dyn_cast<InsertValueInst>(val)) {
    res = convertInsertValue(iv, fixpt);
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
  
  if (res && res != Unsupported && !(res->getType()->isVoidTy()) && !hasInfo(res)) {
    if (isFloatType(val->getType()) && !valueInfo(val)->noTypeConversion) {
      std::string tmpstore;
      raw_string_ostream tmp(tmpstore);
      if (res->hasName())
        tmp << res->getName().str() << ".";
      else if (val->hasName())
        tmp << val->getName().str() << ".";
      tmp << *fixpt;
      res->setName(tmp.str());
    } else if (valueInfo(val)->noTypeConversion) {
      std::string tmpstore;
      raw_string_ostream tmp(tmpstore);
      if (res->hasName())
        tmp << res->getName().str() << ".";
      else if (val->hasName())
        tmp << val->getName().str() << ".";
      tmp << "matchop";
      res->setName(tmp.str());
    }
  }

  return res ? res : ConversionError;
}


Value *FloatToFixed::convertAlloca(AllocaInst *alloca, TypeOverlay *&fixpt)
{
  if (valueInfo(alloca)->noTypeConversion)
    return alloca;
  
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


Value *FloatToFixed::convertLoad(LoadInst *load, TypeOverlay *&fixpt)
{
  Value *ptr = load->getPointerOperand();
  Value *newptr = operandPool[ptr];
  if (newptr == ConversionError)
    return nullptr;
  if (!newptr)
    return Unsupported;
  
  if (isConvertedFixedPoint(newptr)) {
    fixpt = fixPType(newptr);

    LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
      load->getAlignment(), load->getOrdering(), load->getSyncScopeID());
    newinst->insertAfter(load);
    if (valueInfo(load)->noTypeConversion) {
      assert(newinst->getType()->isIntegerTy() && "DTA bug; improperly tagged struct/pointer!");
      return genConvertFixToFloat(newinst, cast<FixedPointTypeOverlay>(fixPType(newptr)), load->getType());
    }
    return newinst;
  }
  
  return Unsupported;
}


Value *FloatToFixed::convertStore(StoreInst *store)
{
  Value *ptr = store->getPointerOperand();
  Value *val = store->getValueOperand();
  
  Value *newptr = matchOp(ptr);
  if (!newptr)
    return nullptr;
  
  Value *newval;
  Type *peltype = newptr->getType()->getPointerElementType();
  if (isFloatingPointToConvert(val)) {
    /* value is converted (thus we can match it) */
    
    if (isConvertedFixedPoint(newptr)) {
      TypeOverlay *valtype = fixPType(newptr);
      
      if (peltype->isPointerTy()) {
        /* store <value ptr> into <value ptr> pointer; both are converted
         * so everything is fine and there is nothing special to do.
         * Only logging type mismatches because we trust DTA has done its job */
        newval = matchOp(val);
        if (!(fixPType(newval) == valtype))
          LLVM_DEBUG(dbgs() << "unsolvable fixp type mismatch between store dest and src!\n");
      } else {
        /* best case: store <value> into <value> pointer */
        valtype = fixPType(newptr);
        newval = translateOrMatchOperandAndType(val, valtype, store);
      }
      
    } else {
      /* store fixp <value ptr?> into original <value ptr?> pointer
       * try to match the stored value if possible */
      newval = fallbackMatchValue(val, peltype);
      if (!newval)
        return Unsupported;
    }
      
  } else {
    /* value is not converted */
    if (isConvertedFixedPoint(newptr)) {
      TypeOverlay *valtype = fixPType(newptr);
      
      /* the value to store is not converted but the pointer is */
      if (peltype->isIntegerTy()) {
        /* value is not a pointer; we can convert it to fixed point */
        newval = genConvertFloatToFix(val, cast<FixedPointTypeOverlay>(valtype));
      } else {
        /* value unconverted ptr; dest is converted ptr
         * would be an error; remove this as soon as it is not needed anymore */
        LLVM_DEBUG(dbgs()<< "[Store] HACK: bitcasting operands of wrong type to new type\n");
        BitCastInst *bc = new BitCastInst(val, peltype);
        cpMetaData(bc, val);
        bc->insertBefore(store);
        newval = bc;
      }
    } else {
      /* nothing is converted, just matchop */
      newval = matchOp(val);
    }
  }
  
  if (!newval)
    return nullptr;
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSyncScopeID());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertGep(GetElementPtrInst *gep, TypeOverlay *&fixpt)
{
  IRBuilder <> builder (gep);
  Value *newval = matchOp(gep->getPointerOperand());
  if (!newval)
    return valueInfo(gep)->noTypeConversion ? Unsupported : nullptr;
  
  if (!isConvertedFixedPoint(newval)) {
    /* just replace the arguments, they should stay the same type */
    return Unsupported;
  }
  
  TypeOverlay *tempFixpt = fixPType(newval);

  Type *type = gep->getPointerOperand()->getType();
  fixpt = tempFixpt->unwrapIndexList(type, gep->indices());
  
  /* if conversion is disabled, we can extract values that didn't get a type change,
   * but we cannot extract values that didn't */
  if (valueInfo(gep)->noTypeConversion && !fixpt->isRecursivelyVoid())
    return Unsupported;

  std::vector<Value*> idxlist(gep->indices().begin(), gep->indices().end());
  return builder.CreateInBoundsGEP(newval, idxlist);
}


Value *FloatToFixed::convertExtractValue(ExtractValueInst *exv, TypeOverlay *&fixpt)
{
  if (valueInfo(exv)->noTypeConversion)
    return Unsupported;
  
  IRBuilder<> builder(exv);
  
  Value *oldval = exv->getAggregateOperand();
  Value *newval = matchOp(oldval);
  if (!newval)
    return nullptr;
  
  TypeOverlay *baset = fixPType(newval)->unwrapIndexList(oldval->getType(), exv->getIndices());

  std::vector<unsigned> idxlist(exv->indices().begin(), exv->indices().end());
  Value *newi = builder.CreateExtractValue(newval, idxlist);
  if (!baset->isVoid() && newi->getType()->isIntegerTy())
    return genConvertFixedToFixed(newi, cast<FixedPointTypeOverlay>(baset), cast<FixedPointTypeOverlay>(fixpt));
  fixpt = baset;
  return newi;
}


Value *FloatToFixed::convertInsertValue(InsertValueInst *inv, TypeOverlay *&fixpt)
{
  if (valueInfo(inv)->noTypeConversion)
    return Unsupported;
  
  IRBuilder<> builder(inv);
  
  Value *oldAggVal = inv->getAggregateOperand();
  Value *newAggVal = matchOp(oldAggVal);
  if (!newAggVal)
    return nullptr;
  
  TypeOverlay *baset = fixPType(newAggVal)->unwrapIndexList(oldAggVal->getType(), inv->getIndices());
  
  Value *oldInsertVal = inv->getInsertedValueOperand();
  Value *newInsertVal;
  if (!baset->isVoid())
    newInsertVal = translateOrMatchOperandAndType(oldInsertVal, baset);
  else
    newInsertVal = oldInsertVal;
  if (!newInsertVal)
    return nullptr;
  
  fixpt = fixPType(newAggVal);
  std::vector<unsigned> idxlist(inv->indices().begin(), inv->indices().end());
  return builder.CreateInsertValue(newAggVal, newInsertVal, idxlist);
}


Value *FloatToFixed::convertPhi(PHINode *phi, TypeOverlay *&fixpt)
{
  if (!phi->getType()->isFloatingPointTy() || valueInfo(phi)->noTypeConversion) {
    /* in the conversion chain the floating point number was converted to
     * an int at some point; we just upgrade the incoming values in place */

    /* if all of our incoming values were not converted, we want to propagate
     * that information across the phi. If at least one of them was converted
     * the phi is converted as well; otherwise it is not. */
    bool donesomething = false;

    for (int i=0; i<phi->getNumIncomingValues(); i++) {
      Value *thisval = phi->getIncomingValue(i);
      Value *newval = fallbackMatchValue(thisval, thisval->getType(), phi);
      if (newval && newval != ConversionError) {
        phi->setIncomingValue(i, newval);
        donesomething = true;
      }
    }
    return donesomething ? phi : nullptr;
  }

  /* if we have to do a type change, create a new phi node. The new type is for
   * sure that of a fixed point value; because the original type was a float
   * and thus all of its incoming values were floats */
  PHINode *newphi = PHINode::Create(cast<UniformTypeOverlay>(fixpt)->getBaseLLVMType(phi->getContext()),
    phi->getNumIncomingValues());

  for (int i=0; i<phi->getNumIncomingValues(); i++) {
    Value *thisval = phi->getIncomingValue(i);
    BasicBlock *thisbb = phi->getIncomingBlock(i);
    Value *newval = translateOrMatchOperandAndType(thisval, fixpt, thisbb->getTerminator());
    if (!newval) {
      delete newphi;
      return nullptr;
    }
    Instruction *inst2 = dyn_cast<Instruction>(newval);
    if (inst2) {
      LLVM_DEBUG(dbgs() << "warning: new phi value " << *inst2 << " not coming from the obviously correct BB\n");
    }
    newphi->addIncoming(newval, thisbb);
  }
  newphi->insertAfter(phi);
  return newphi;
}


Value *FloatToFixed::convertSelect(SelectInst *sel, TypeOverlay *&fixpt)
{
  if (!isFloatingPointToConvert(sel))
    return Unsupported;
  
  /* the condition is always a bool (i1) or a vector of bools */
  Value *newcond = matchOp(sel->getCondition());

  /* otherwise create a new one */
  Value *newtruev = translateOrMatchAnyOperandAndType(sel->getTrueValue(), fixpt, sel);
  Value *newfalsev = translateOrMatchAnyOperandAndType(sel->getFalseValue(), fixpt, sel);
  if (!newtruev || !newfalsev || !newcond)
    return nullptr;

  SelectInst *newsel = SelectInst::Create(newcond, newtruev, newfalsev);
  newsel->insertAfter(sel);
  return newsel;
}


Value *FloatToFixed::convertCall(CallSite *call, TypeOverlay *&fixpt)
{
  /* If the function return a float the new return type will be a fix point of type fixpt,
   * otherwise the return type is left unchanged.*/
  Function *oldF = call->getCalledFunction();

  if (isSpecialFunction(oldF))
    return Unsupported;
  
  Function *newF = functionPool[oldF];
  if (!newF) {
    LLVM_DEBUG(dbgs() << "[Info] no function clone for instruction" << *(call->getInstruction()) << ", engaging fallback\n");
    return Unsupported;
  }
  
  LLVM_DEBUG(dbgs() << *(call->getInstruction()) <<  " will use converted function " <<
               newF->getName() << " " << *newF->getType() << "\n";);

  std::vector<Value*> convArgs;
  std::vector<Type*> typeArgs;
  std::vector<std::pair<int, TypeOverlay *>> fixArgs; //for match right function
  
  if (isFloatType(oldF->getReturnType())) {
    fixArgs.push_back(std::pair<int, TypeOverlay *>(-1, fixpt)); //ret value in signature
  }

  int i=0;
  auto *call_arg = call->arg_begin();
  auto *f_arg = newF->arg_begin();
  while (call_arg != call->arg_end()) {
    Value *thisArgument;
    
    if (!hasInfo(f_arg)) {
      if (!hasInfo(*call_arg)) {
        thisArgument = *call_arg;
      } else {
        thisArgument = fallbackMatchValue(*call_arg, f_arg->getType());
      }
      
    } else if (hasInfo(*call_arg) && valueInfo(*call_arg)->noTypeConversion == false) {
      TypeOverlay *argfpt, *funfpt;
      argfpt = fixPType(*call_arg);
      funfpt = fixPType(&(*f_arg));
      if (!(argfpt == funfpt)) {
        LLVM_DEBUG(dbgs() << "CALL: fixed point type mismatch in actual argument " << i << " (" << *f_arg << ") vs. formal argument\n");
        LLVM_DEBUG(dbgs() << "      (actual " << *argfpt << ", vs. formal " << *funfpt << ")\n");
        LLVM_DEBUG(dbgs() << "      making an attempt to ignore the issue because mem2reg can interfere\n");
      }
      thisArgument = translateOrMatchAnyOperandAndType(*call_arg, funfpt, call->getInstruction());
      fixArgs.push_back(std::pair<int, TypeOverlay *>(i, funfpt));
      
    } else {
      TypeOverlay *funfpt;
      funfpt = fixPType(&(*f_arg));
      LLVM_DEBUG(dbgs() << "CALL: formal argument " << i << " (" << *f_arg << ") converted but not actual argument\n");
      LLVM_DEBUG(dbgs() << "      making an attempt to ignore the issue because mem2reg can interfere\n");
      thisArgument = translateOrMatchAnyOperandAndType(*call_arg, funfpt, call->getInstruction());
    }
    
    if (!thisArgument) {
      LLVM_DEBUG(dbgs() << "CALL: match of argument " << i << " (" << *f_arg << ") failed\n");
      return Unsupported;
    }
    
    convArgs.push_back(thisArgument);
    typeArgs.push_back(thisArgument->getType());
    
    if (convArgs[i]->getType() != f_arg->getType()) {
      LLVM_DEBUG(dbgs() << "CALL: type mismatch in actual argument " << i << " (" << *f_arg << ") vs. formal argument\n");
      return nullptr;
    }
    
    i++;
    call_arg++;
    f_arg++;
  }

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
  }
  
  assert(false && "Unknown CallSite type");
  return Unsupported;
}


Value *FloatToFixed::convertRet(ReturnInst *ret, TypeOverlay *&fixpt)
{
  Value *oldv = ret->getReturnValue();
  
  if (!oldv) // AKA return void
    return ret;
  
  if (!isFloatingPointToConvert(ret) || valueInfo(ret)->noTypeConversion) {
    //if return an int we shouldn't return a fix point, go into fallback
    return Unsupported;
  }
  
  Function* f = dyn_cast<Function>(ret->getParent()->getParent());
  Value *v = translateOrMatchAnyOperandAndType(oldv, fixpt);
  
  // check return type
  if (f->getReturnType() != v->getType())
    return nullptr;
  
  ret->setOperand(0,v);
  return ret;
}


Value *FloatToFixed::convertBinOp(Instruction *instr, TypeOverlay *&fixpt_gen)
{
  /*le istruzioni Instruction::
    [Add,Sub,Mul,SDiv,UDiv,SRem,URem,Shl,LShr,AShr,And,Or,Xor]
    vengono gestite dalla fallback e non in questa funzione */
  if (!instr->getType()->isFloatingPointTy() || valueInfo(instr)->noTypeConversion)
    return Unsupported;
  UniformTypeOverlay *fixpt = cast<UniformTypeOverlay>(fixpt_gen);
  
  int opc = instr->getOpcode();
  Value *fixop;

  if (opc == Instruction::FAdd || opc == Instruction::FSub || opc == Instruction::FRem) {
    Value *val1 = translateOrMatchOperandAndType(instr->getOperand(0), fixpt, instr);
    Value *val2 = translateOrMatchOperandAndType(instr->getOperand(1), fixpt, instr);
    if (!val1 || !val2)
      return nullptr;
    
    if (opc == Instruction::FAdd) {
      fixop = fixpt->genAdd(val1, fixpt, val2, fixpt, instr);
    } else if (opc == Instruction::FSub) {
      fixop = fixpt->genSub(val1, fixpt, val2, fixpt, instr);
    } else /* if (opc == Instruction::FRem) */ {
      fixop = fixpt->genRem(val1, fixpt, val2, fixpt, instr);
    }

    updateConstTypeMetadata(fixop, 0U, fixpt);
    updateConstTypeMetadata(fixop, 1U, fixpt);
    return fixop;

  } else if (opc == Instruction::FMul) {
    TypeOverlay *intype1 = fixpt, *intype2 = fixpt;
    Value *val1 = translateOrMatchOperand(instr->getOperand(0), intype1, instr, TypeMatchPolicy::RangeOverHintMaxInt);
    Value *val2 = translateOrMatchOperand(instr->getOperand(1), intype2, instr, TypeMatchPolicy::RangeOverHintMaxInt);
    if (!val1 || !val2)
      return nullptr;
      
    return fixpt->genMul(val1, cast<UniformTypeOverlay>(intype1), val2, cast<UniformTypeOverlay>(intype2), instr);
    return fixop;
    
  } else if (opc == Instruction::FDiv) {
    // TODO: fix by using HintOverRange when it is actually implemented
    TypeOverlay *intype1 = fixpt, *intype2 = fixpt;
    Value *val1 = translateOrMatchOperand(instr->getOperand(0), intype1, instr, TypeMatchPolicy::RangeOverHintMaxFrac);
    Value *val2 = translateOrMatchOperand(instr->getOperand(1), intype2, instr, TypeMatchPolicy::RangeOverHintMaxInt);
    if (!val1 || !val2)
      return nullptr;
      
    return fixpt->genDiv(val1, cast<UniformTypeOverlay>(intype1), val2, cast<UniformTypeOverlay>(intype2), instr);
  }
  
  return Unsupported;
}


Value *FloatToFixed::convertCmp(FCmpInst *fcmp)
{
  Value *op1 = fcmp->getOperand(0);
  Value *op2 = fcmp->getOperand(1);
  
  TypeOverlay *t1, *t2;
  bool hasinfo1 = hasInfo(op1), hasinfo2 = hasInfo(op2);
  if (hasinfo1 && hasinfo2) {
    t1 = cast<UniformTypeOverlay>(fixPType(op1));
    t2 = cast<UniformTypeOverlay>(fixPType(op2));
  } else if (hasinfo1) {
    t1 = cast<UniformTypeOverlay>(fixPType(op1));
    t2 = t1;
  } else if (hasinfo2) {
    t2 = cast<UniformTypeOverlay>(fixPType(op2));
    t1 = t2;
  } else {
    /* Both operands are not converted... then what are we even doing here? */
    LLVM_DEBUG(dbgs() << "scheduled conversion of fcmp, but no operands are converted?\n");
    return fcmp;
  }
  Value *matchop1 = translateOrMatchOperand(op1, t1, fcmp);
  Value *matchop2 = translateOrMatchOperand(op2, t2, fcmp);
  
  UniformTypeOverlay *matcht1 = cast<UniformTypeOverlay>(t1);
  UniformTypeOverlay *matcht2 = cast<UniformTypeOverlay>(t2);
  
  if (isa<FixedPointTypeOverlay>(matcht1)) {
    return matcht1->genCmp(fcmp->getPredicate(), matchop1, matchop2, matcht2, fcmp);
  }
  return matcht2->genCmp(fcmp->getInversePredicate(), matchop2, matchop1, matcht1, fcmp);
}


Value *FloatToFixed::convertCast(CastInst *cast, TypeOverlay *fixpt)
{
  /* le istruzioni Instruction::
   * - [FPToSI,FPToUI,SIToFP,UIToFP] vengono gestite qui
   * - [Trunc,ZExt,SExt] vengono gestite dalla fallback e non qui
   * - [PtrToInt,IntToPtr,BitCast,AddrSpaceCast] potrebbero portare errori */

  IRBuilder<> builder(cast->getNextNode());
  Value *operand = cast->getOperand(0);
  
  if (valueInfo(cast)->noTypeConversion)
    return Unsupported;
  
  if (BitCastInst *bc = dyn_cast<BitCastInst>(cast)) {
    Value *newOperand = operandPool[operand];
    Type *newType = getLLVMFixedPointTypeForFloatType(bc->getDestTy(), fixpt);
    if (newOperand && newOperand!=ConversionError){
      return builder.CreateBitCast(newOperand, newType);
    } else {
      return builder.CreateBitCast(operand, newType);
    }
  }
  
  if (operand->getType()->isFloatingPointTy()) {
    /* fptosi, fptoui, fptrunc, fpext */
    if (cast->getOpcode() == Instruction::FPToSI) {
      return translateOrMatchOperandAndType(operand, TypeOverlay::get(this, cast->getType(), true), cast);

    } else if (cast->getOpcode() == Instruction::FPToUI) {
      return translateOrMatchOperandAndType(operand, TypeOverlay::get(this, cast->getType(), false), cast);

    } else if (cast->getOpcode() == Instruction::FPTrunc ||
               cast->getOpcode() == Instruction::FPExt) {
      return translateOrMatchOperandAndType(operand, fixpt, cast);
    }
  } else {
    /* sitofp, uitofp */
    Value *val = matchOp(operand);
    
    if (cast->getOpcode() == Instruction::SIToFP) {
      return genConvertFixedToFixed(val, FixedPointTypeOverlay::get(this, val->getType(), true), dyn_cast<FixedPointTypeOverlay>(fixpt), cast);

    } else if (cast->getOpcode() == Instruction::UIToFP) {
      return genConvertFixedToFixed(val, FixedPointTypeOverlay::get(this, val->getType(), false), dyn_cast<FixedPointTypeOverlay>(fixpt), cast);
    }
  }

  return Unsupported;
}


Value *FloatToFixed::fallback(Instruction *unsupp, TypeOverlay *&fixpt)
{
  Value *fallval;
  Value *fixval;
  std::vector<Value *> newops;

  LLVM_DEBUG(dbgs() << "[Fallback] attempt to wrap not supported operation:\n" << *unsupp << "\n");
  FallbackCount++;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);
    fixval = fallbackMatchValue(fallval, fallval->getType(), unsupp);

    if (fixval) {
      LLVM_DEBUG(dbgs() << "  Substituted operand number : " << i+1 << " of " << n << "\n");
      newops.push_back(fixval);
    } else {
      newops.push_back(fallval);
    }
  }
  
  Instruction *tmp;
  if (valueInfo(unsupp)->noTypeConversion == false && !unsupp->isTerminator()) {
    tmp = unsupp->clone();
    if (!tmp->getType()->isVoidTy())
      tmp->setName(unsupp->getName() + ".flt");
    tmp->insertAfter(unsupp);
  } else {
    tmp = unsupp;
  }
  
  for (int i=0, n=tmp->getNumOperands(); i<n; i++) {
    tmp->setOperand(i, newops[i]);
  }
  LLVM_DEBUG(dbgs() << "  mutated operands to:\n" << *tmp << "\n");
  if (tmp->getType()->isFloatingPointTy() && valueInfo(unsupp)->noTypeConversion == false) {
    FixedPointTypeOverlay *FXT = cast<FixedPointTypeOverlay>(fixpt);
    Value *fallbackv = genConvertFloatToFix(tmp, FXT, getFirstInsertionPointAfter(tmp));
    if (tmp->hasName())
      fallbackv->setName(tmp->getName() + ".fallback");
    return fallbackv;
  }
  return tmp;
}

