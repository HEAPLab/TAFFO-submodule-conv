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
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    return convertAlloca(m, alloca);
  }
  return nullptr;
}


Value *FloatToFixed::convertAlloca(Module& m, AllocaInst *alloca)
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
  
  AllocaInst *newinst = dyn_cast<AllocaInst>(alloca->clone());
  newinst->setAllocatedType(Type::getIntNTy(newinst->getContext(), bitsAmt));
  newinst->setName("__" + alloca->getName() + "_fixp");
  newinst->insertAfter(alloca);
  return newinst;
}


Value *FloatToFixed::genConvertFloatToFix(Module& m, Value *flt)
{
  Instruction *i = dyn_cast<Instruction>(flt);
  if (!i)
    return nullptr;
  
  IRBuilder<> builder(i);
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFPToSI(
    builder.CreateMul(
      ConstantFP::get(flt->getType(), twoebits),
      flt),
    Type::getIntNTy(i->getContext(), bitsAmt));
}


Value *FloatToFixed::genConvertFixToFloat(Module& m, Value *fix, Type *destt)
{
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;
  
  IRBuilder<> builder(i);
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFDiv(
    builder.CreateSIToFP(
      fix, destt),
    ConstantFP::get(destt, twoebits));
}


