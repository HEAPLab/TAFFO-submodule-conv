#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>
#include "LLVMFloatToFixedPass.h"

using namespace llvm;
using namespace flttofix;


void FloatToFixed::performConversion(Module& m, const std::vector<Value*>& q)
{
  DenseMap<Value *, Value *> convertedPool;
  bool convcomplete = true;

  for (Value *v: q) {
    Value *newv = convertSingleValue(m, convertedPool, v);
    if (newv) {
      convertedPool.insert({v, newv});
    } else {
      errs() << "warning: ";
      v->print(errs());
      errs() << " not converted\n";
      convcomplete = false;
    }
  }

  if (convcomplete) {
    for (Value *v: q) {
      if (auto *i = dyn_cast<Instruction>(v))
        i->removeFromParent();
    }
  }
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, DenseMap<Value *, Value *>& operandPool, Value *val)
{
  Value *res = nullptr;
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    res = convertAlloca(alloca);
  } else if (LoadInst *load = dyn_cast<LoadInst>(val)) {
    res = convertLoad(operandPool, load);
  } else if (Instruction *unsupp = dyn_cast<Instruction>(val)){
    res = fallback(m,operandPool,unsupp);
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
  if (!newptr) {
    return nullptr;
  }

  LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
    load->getAlignment(), load->getOrdering(), load->getSynchScope());
  newinst->insertAfter(load);
  return newinst;
}


Value *FloatToFixed::fallback(Module &m,DenseMap<Value *, Value *>& op, Instruction *unsupp)
{
  Instruction *newinst = unsupp->clone();
  Value *fallval;
  Instruction *fixval;
  bool substitute = false;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);
    //se è stato precedentemente sostituito e non è un puntatore
    if (op[fallval] && !fallval->getType()->isPointerTy()) {
      /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
        il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
        Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
      fixval = fallval->getType()->isFloatingPointTy()
        ? dyn_cast<Instruction>(genConvertFixToFloat(op[fallval],fallval->getType()))
        : dyn_cast<Instruction>(op[fallval]);

      errs() << "[Fallback] Substituted operand number : " << i+1 << " of " << n << "\n";
      newinst->setOperand(i,fixval);
      substitute=true;
    }
  }

  if (substitute) {
    if (unsupp->hasName()) {
      newinst->setName(unsupp->getName() + "_fallback");
    }
    newinst->insertAfter(unsupp);
    errs() << "[Fallback] Not supported operation :" << *unsupp <<" converted to :" << *newinst << " \n";
    if (unsupp->getType()->isFloatingPointTy()) {
      return genConvertFloatToFix(newinst);
    }
    return newinst;
  }
  else {
    delete newinst;
    return nullptr;
  }
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt)
{
  Instruction *i = dyn_cast<Instruction>(flt);
  if (!i)
    return nullptr;

  IRBuilder<> builder(i->getNextNode());
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFPToSI(
    builder.CreateFMul(
      ConstantFP::get(flt->getType(), twoebits),
      flt),
    Type::getIntNTy(i->getContext(), bitsAmt));
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, Type *destt)
{
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;

  IRBuilder<> builder(i->getNextNode());
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFDiv(
    builder.CreateSIToFP(
      fix, destt),
    ConstantFP::get(destt, twoebits));
}


