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


void FloatToFixed::performConversion(
  Module& m,
  std::vector<Value*>& q)
{
  
  for (auto i = q.begin(); i != q.end();) {
    Value *v = *i;
    
    if (CallInst *anno = dyn_cast<CallInst>(v)) {
      if (anno->getCalledFunction()) {
        if (anno->getCalledFunction()->getName() == "llvm.var.annotation") {
          anno->eraseFromParent();
          i = q.erase(i);
          continue;
        }
      }
    }
    
    Value *newv = convertSingleValue(m, v);
    if (newv && newv != ConversionError) {
      operandPool.insert({v, newv});
    } else {
      DEBUG(dbgs() << "warning: ";
            v->print(dbgs());
            dbgs() << " not converted\n";);
      
      if (newv)
        operandPool.insert({v, newv});
    }
    
    i++;
  }
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, Value *val)
{
  Value *res = Unsupported;
  
  if (Constant *con = dyn_cast<Constant>(val)) {
    res = convertConstant(con);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) {
    res = convertInstruction(m, instr);
  }
  
  return res ? res : ConversionError;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(Value *val, Instruction *ip)
{
  Value *res = operandPool[val];
  if (res) {
    if (res != ConversionError)
      /* the value has been successfully converted in a previous step */
      return res;
    else
      /* the value should have been converted but it hasn't; bail out */
      return nullptr;
  }

  if (!val->getType()->isPointerTy() && !val->getType()->isFloatingPointTy())
    /* doesn't need to be converted; return as is */
    return val;

  return genConvertFloatToFix(val, ip);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt, Instruction *ip)
{
  if (Constant *c = dyn_cast<Constant>(flt)) {
    return convertConstant(c);
  }

  FloatToFixCount++;

  if (Instruction *i = dyn_cast<Instruction>(flt))
    ip = i->getNextNode();
  assert(ip && "ip is mandatory if not passing an instruction/constant value");
  
  if (!flt->getType()->isFloatingPointTy()) {
    DEBUG(errs() << "can't wrap-convert to fixp non float value ";
          flt->print(errs());
          errs() << "\n");
    return nullptr;
  }
  
  IRBuilder<> builder(ip);
  
  /* insert new instructions before ip */
  if (SIToFPInst *instr = dyn_cast<SIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return builder.CreateShl(intparam, fracBitsAmt);
  } else if (UIToFPInst *instr = dyn_cast<UIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return builder.CreateShl(intparam, fracBitsAmt);
  } else {
    double twoebits = pow(2.0, fracBitsAmt);
    return builder.CreateFPToSI(
      builder.CreateFMul(
        ConstantFP::get(flt->getType(), twoebits),
        flt),
      getFixedPointType(flt->getContext()));
  }
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, Type *destt)
{
  dbgs() << "******** trace: genConvertFixToFloat ";
  fix->print(dbgs());
  dbgs() << " ";
  destt->print(dbgs());
  dbgs() << "\n";
  
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;
  FixToFloatCount++;
  
  if (!fix->getType()->isIntegerTy()) {
    DEBUG(errs() << "can't wrap-convert to flt non integer value ";
          fix->print(errs());
          errs() << "\n");
    return nullptr;
  }

  IRBuilder<> builder(i->getNextNode());
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFDiv(
    builder.CreateSIToFP(
      fix, destt),
    ConstantFP::get(destt, twoebits));
}


Type *FloatToFixed::getFixedPointTypeForFloatType(Type *srct)
{
  if (srct->isPointerTy()) {
    Type *enc = getFixedPointTypeForFloatType(srct->getPointerElementType());
    if (enc)
      return enc->getPointerTo();
    return nullptr;
    
  } else if (srct->isArrayTy()) {
    int nel = srct->getArrayNumElements();
    Type *enc = getFixedPointTypeForFloatType(srct->getArrayElementType());
    if (enc)
      return ArrayType::get(enc, nel);
    return nullptr;
    
  } else if (srct->isFloatingPointTy()) {
    return getFixedPointType(srct->getContext());
    
  }
  DEBUG(srct->dump());
  DEBUG(dbgs() << "getFixedPointTypeForFloatType given a non-float type\n");
  return srct;
}


Type *FloatToFixed::getFixedPointType(LLVMContext &ctxt)
{
  return Type::getIntNTy(ctxt, bitsAmt);
}

