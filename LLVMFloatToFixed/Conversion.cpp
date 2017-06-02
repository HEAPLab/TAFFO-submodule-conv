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


#include <cmath>
#include <cassert>
#include "LLVMFloatToFixedPass.h"

using namespace llvm;
using namespace flttofix;


Value *ConversionError = (Value *)(&ConversionError);
Value *Unsupported = (Value *)(&Unsupported);


void FloatToFixed::performConversion(Module& m, const std::vector<Value*>& q)
{
  DenseMap<Value *, Value *> convertedPool;

  for (Value *v: q) {
    Value *newv = convertSingleValue(m, convertedPool, v);
    if (newv && newv != ConversionError) {
      convertedPool.insert({v, newv});
    } else {
      DEBUG(errs() << "warning: ";
            v->print(errs());
            errs() << " not converted\n";);

      convertedPool.insert({v, ConversionError});
    }
  }

  /* remove all successfully converted stores. MAY PRODUCE INCORRECT RESULTS */
  //TODO: better logic here!!
  for (Value *v: q) {
    auto *i = dyn_cast<StoreInst>(v);
    auto *convi = convertedPool[v];
    if (convi && convi != ConversionError && i)
      i->eraseFromParent();
  }
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, DenseMap<Value *, Value *>& operandPool, Value *val)
{
  Value *res = Unsupported;
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    res = convertAlloca(alloca);
  } else if (LoadInst *load = dyn_cast<LoadInst>(val)) {
    res = convertLoad(operandPool, load);
  } else if (StoreInst *store = dyn_cast<StoreInst>(val)) {
    res = convertStore(operandPool, store);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) { //llvm/include/llvm/IR/Instruction.def for more info
    if (instr->isBinaryOp()) {
      res = convertBinOp(operandPool,instr);
    } else if (instr->isCast()){
      /*le istruzioni Instruction::
        [Trunc,ZExt,SExt,FPToUI,FPToSI,UIToFP,SIToFP,FPTrunc,FPExt]
        vengono gestite dalla fallback e non qui
        [PtrToInt,IntToPtr,BitCast,AddrSpaceCast] potrebbero portare errori*/;
    } else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(val)) {
      res = convertCmp(operandPool,fcmp);
    }
  }

  if (res==Unsupported) {
    res = fallback(operandPool,dyn_cast<Instruction>(val));
  }

  return res;
}


Value *FloatToFixed::convertAlloca(AllocaInst *alloca)
{
  Type *prevt = alloca->getAllocatedType();
  if (!prevt->isFloatingPointTy()) {
    errs() << *alloca << " does not directly allocate a float\n";
    return nullptr;
  }

  Value *as = alloca->getArraySize();
  ConstantInt *ci = dyn_cast<ConstantInt>(as);
  if (!ci || ci->getSExtValue() != 1) {
    errs() << *alloca << " is not a single value!\n";
    return nullptr;
  }

  Type *alloct = Type::getIntNTy(alloca->getContext(), bitsAmt);
  AllocaInst *newinst = new AllocaInst(alloct, as, alloca->getAlignment(),
    "__" + alloca->getName() + "_fixp");
  newinst->setUsedWithInAlloca(alloca->isUsedWithInAlloca());
  newinst->setSwiftError(alloca->isSwiftError());
  newinst->insertAfter(alloca);
  return newinst;
}


Value *FloatToFixed::convertLoad(DenseMap<Value *, Value *>& op, LoadInst *load)
{
  Value *ptr = load->getPointerOperand();
  Value *newptr = op[ptr];
  if (newptr == ConversionError)
    return nullptr;
  assert(newptr && "a load can't be in the conversion queue just because");

  LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
    load->getAlignment(), load->getOrdering(), load->getSynchScope());
  newinst->insertAfter(load);
  return newinst;
}


Value *FloatToFixed::convertStore(DenseMap<Value *, Value *>& op, StoreInst *store)
{
  Value *ptr = store->getPointerOperand();
  Value *newptr = op[ptr];
  if (!newptr)
    newptr = ptr;
  else if (newptr == ConversionError)
    return nullptr;

  Value *val = store->getValueOperand();
  Value *newval = translateOrMatchOperand(op, val);
  if (!newval)
    return nullptr;

  if (newval->getType() != cast<PointerType>(newptr->getType())->getElementType()) {
    //se l'area di memoria non è float , riporto il valore in float per memorizzarlo correttamente
    newval = genConvertFixToFloat(newval,cast<PointerType>(newptr->getType())->getElementType());
  }
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSynchScope());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertBinOp(DenseMap<Value *, Value *>& op, Instruction *instr)
{
  IRBuilder<> builder(instr->getNextNode());
  Instruction::BinaryOps ty;
  int opc = instr->getOpcode();
  /*le istruzioni Instruction::
    [Add,Sub,Mul,SDiv,UDiv,SRem,URem,Shl,LShr,AShr,And,Or,Xor]
    vengono gestite dalla fallback e non in questa funzione */
  if (opc == Instruction::FAdd) {
    ty = Instruction::Add;
  } else if (opc == Instruction::FSub) {
    ty = Instruction::Sub;
  } else if (opc == Instruction::FMul) {
    ty = Instruction::Mul;
  } else if (opc == Instruction::FDiv) {
    ty = Instruction::SDiv;
  } else if (opc == Instruction::FRem) {
    ty = Instruction::SRem;
  } else {
    return Unsupported;
  }

  Value *val1 = translateOrMatchOperand(op,instr->getOperand(0));
  Value *val2 = translateOrMatchOperand(op,instr->getOperand(1));

  return val1 && val2
    ? builder.CreateBinOp(ty,val1,val2)
    : nullptr;
}


Value *FloatToFixed::convertCmp(DenseMap<Value *, Value *>& op, FCmpInst *fcmp)
{
  IRBuilder<> builder (fcmp->getNextNode());
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
    ty = CmpInst::ICMP_SGT;
  } else if (pr == CmpInst::FCMP_OGE) {
    ty = CmpInst::ICMP_SGE;
  } else if (pr == CmpInst::FCMP_OLE) {
    ty = CmpInst::ICMP_SLE;
  } else if (pr == CmpInst::FCMP_OLT) {
    ty = CmpInst::ICMP_SLT;
  } else if (pr == CmpInst::FCMP_ORD) {
    ;
    //TODO gestione NaN
  } else if (pr == CmpInst::FCMP_TRUE) {
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

  Value *val1 = translateOrMatchOperand(op,fcmp->getOperand(0));
  Value *val2 = translateOrMatchOperand(op,fcmp->getOperand(1));

  return val1 && val2
    ? builder.CreateICmp(ty,val1,val2)
    : nullptr;
}


Value *FloatToFixed::fallback(DenseMap<Value *, Value *>& op, Instruction *unsupp)
{
  Value *fallval;
  Instruction *fixval;
  std::vector<Value *> newops;

  DEBUG(errs() << "[Fallback] attempt to wrap not supported operation:\n" << *unsupp << "\n");
  FallbackCount++;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);

    Value *cvtfallval = op[fallval];
    if (cvtfallval == ConversionError || fallval->getType()->isPointerTy()) {
      DEBUG(errs() << "  bail out on missing operand " << i+1 << " of " << n << "\n");
      return nullptr;
    }

    //se è stato precedentemente sostituito e non è un puntatore
    if (cvtfallval) {
      /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
        il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
        Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
      fixval = fallval->getType()->isFloatingPointTy()
        ? dyn_cast<Instruction>(genConvertFixToFloat(cvtfallval,fallval->getType()))
        : dyn_cast<Instruction>(cvtfallval);

      DEBUG(errs() << "  Substituted operand number : " << i+1 << " of " << n << "\n");
      newops.push_back(fixval);
    } else {
      newops.push_back(fallval);
    }
  }

  for (int i=0, n=unsupp->getNumOperands(); i<n; i++) {
    unsupp->setOperand(i, newops[i]);
  }
  DEBUG(errs() << "  mutated operands to:\n" << *unsupp << "\n");
  if (unsupp->getType()->isFloatingPointTy()) {
    Value *fallbackv = genConvertFloatToFix(unsupp);
    if (unsupp->hasName())
      fallbackv->setName(unsupp->getName() + "_fallback");
    return fallbackv;
  }
  return unsupp;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(DenseMap<Value *, Value *>& op, Value *val)
{
  Value *res = op[val];
  if (res) {
    if (res != ConversionError)
      /* the value has been successfully converted in a previous step */
      return res;
    else
      /* the value should have been converted but it hasn't; bail out */
      return nullptr;
  }

  if (!val->getType()->isFloatingPointTy())
    /* doesn't need to be converted; return as is */
    return val;

  return genConvertFloatToFix(val);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt)
{
  if (ConstantFP *fpc = dyn_cast<ConstantFP>(flt)) {
    return convertFloatConstantToFixConstant(fpc);

  } else if (Instruction *i = dyn_cast<Instruction>(flt)) {
    FloatToFixCount++;
    
    IRBuilder<> builder(i->getNextNode());
    double twoebits = pow(2.0, fracBitsAmt);
    return builder.CreateFPToSI(
      builder.CreateFMul(
        ConstantFP::get(flt->getType(), twoebits),
        flt),
      Type::getIntNTy(i->getContext(), bitsAmt));

  }
  return nullptr;
}


Constant *FloatToFixed::convertFloatConstantToFixConstant(ConstantFP *fpc)
{
  bool precise = false;
  APFloat val = fpc->getValueAPF();

  APFloat exp(pow(2.0, fracBitsAmt));
  exp.convert(val.getSemantics(), APFloat::rmTowardNegative, &precise);
  val.multiply(exp, APFloat::rmTowardNegative);

  integerPart fixval;
  val.convertToInteger(&fixval, 64, true, APFloat::rmTowardNegative, &precise);

  Type *intty = Type::getIntNTy(fpc->getType()->getContext(), bitsAmt);
  return ConstantInt::get(intty, fixval, true);
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, Type *destt)
{
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;
  FixToFloatCount++;

  IRBuilder<> builder(i->getNextNode());
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFDiv(
    builder.CreateSIToFP(
      fix, destt),
    ConstantFP::get(destt, twoebits));
}

