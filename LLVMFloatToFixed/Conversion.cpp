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
  std::vector<Value*>& q,
  DenseMap<Value *, Value *>& convertedPool)
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
    
    Value *newv = convertSingleValue(m, convertedPool, v);
    if (newv && newv != ConversionError) {
      convertedPool.insert({v, newv});
    } else {
      DEBUG(dbgs() << "warning: ";
            v->print(dbgs());
            dbgs() << " not converted\n";);
      
      if (newv)
        convertedPool.insert({v, newv});
    }
    
    i++;
  }
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, DenseMap<Value *, Value *>& operandPool, Value *val)
{
  Value *res = Unsupported;
  
  if (Constant *con = dyn_cast<Constant>(val)) {
    res = convertConstant(operandPool, con);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) {
    res = convertInstruction(m, operandPool, instr);
  }
  
  return res ? res : ConversionError;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(DenseMap<Value *, Value *>& op, Value *val, Instruction *ip)
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

  if (!val->getType()->isPointerTy() && !val->getType()->isFloatingPointTy())
    /* doesn't need to be converted; return as is */
    return val;

  return genConvertFloatToFix(op, val, ip);
}


Value *FloatToFixed::genConvertFloatToFix(DenseMap<Value *, Value *>& op, Value *flt, Instruction *ip)
{
  if (Constant *c = dyn_cast<Constant>(flt)) {
    return convertConstant(op, c);
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

  /* insert new instructions before ip */
  IRBuilder<> builder(ip);
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFPToSI(
    builder.CreateFMul(
      ConstantFP::get(flt->getType(), twoebits),
      flt),
    getFixedPointType(flt->getContext()));
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, Type *destt)
{
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
  return nullptr;
}


Type *FloatToFixed::getFixedPointType(LLVMContext &ctxt)
{
  return Type::getIntNTy(ctxt, bitsAmt);
}

