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
    
    Value *newv = convertSingleValue(m, v, info[v].fixpType);
    if (newv && newv != ConversionError) {
      operandPool.insert({v, newv});
      info[newv] = info[v];
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
Value *FloatToFixed::convertSingleValue(Module& m, Value *val, FixedPointType& fixpt)
{
  Value *res = Unsupported;
  
  if (Constant *con = dyn_cast<Constant>(val)) {
    res = convertConstant(con, fixpt);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) {
    res = convertInstruction(m, instr, fixpt);
  }
  
  return res ? res : ConversionError;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(Value *val, FixedPointType& iofixpt, Instruction *ip)
{
  Value *res = operandPool[val];
  if (res) {
    if (res != ConversionError) {
      /* the value has been successfully converted in a previous step */
      iofixpt = fixPType(res);
      return res;
    } else
      /* the value should have been converted but it hasn't; bail out */
      return nullptr;
  }

  assert(val->getType()->isPointerTy() || val->getType()->isFloatingPointTy());
  return genConvertFloatToFix(val, iofixpt, ip);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt, const FixedPointType& fixpt, Instruction *ip)
{
  if (Constant *c = dyn_cast<Constant>(flt)) {
    FixedPointType fixptcopy = fixpt;
    Value *res = convertConstant(c, fixptcopy);
    assert(fixptcopy == fixpt && "why is there a pointer here?");
    return res;
  }

  if (Instruction *i = dyn_cast<Instruction>(flt))
    ip = i->getNextNode();
  assert(ip && "ip is mandatory if not passing an instruction/constant value");
  
  if (!flt->getType()->isFloatingPointTy()) {
    DEBUG(errs() << "can't wrap-convert to fixp non float value ";
          flt->print(errs());
          errs() << "\n");
    return nullptr;
  }
  
  FloatToFixCount++;
  
  IRBuilder<> builder(ip);
  Type *destt = getLLVMFixedPointTypeForFloatType(flt->getType(), fixpt);
  
  /* insert new instructions before ip */
  if (SIToFPInst *instr = dyn_cast<SIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return builder.CreateShl(
              builder.CreateIntCast(intparam, destt, true),
            fixpt.fracBitsAmt);
  } else if (UIToFPInst *instr = dyn_cast<UIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return builder.CreateShl(
              builder.CreateIntCast(intparam, destt, false),
            fixpt.fracBitsAmt);
  } else {
    double twoebits = pow(2.0, fixpt.fracBitsAmt);
    Value *interm = builder.CreateFMul(
          ConstantFP::get(flt->getType(), twoebits),
        flt);
    if (fixpt.isSigned) {
      return builder.CreateFPToSI(interm, destt);
    } else {
      return builder.CreateFPToUI(interm, destt);
    }
  }
}


Value *FloatToFixed::genConvertFixedToFixed(Value *fix, const FixedPointType& srct, const FixedPointType& destt, Instruction *ip)
{
  Type *llvmsrct = fix->getType();
  assert(llvmsrct->isSingleValueType() && "cannot change fixed point format of a pointer");
  assert(llvmsrct->isIntegerTy() && "cannot change fixed point format of a float");

  // Constants are previously converted, fix to fix meaningless
  if (Constant *cons = dyn_cast<Constant>(fix)){
    return cons;
  }
  
  Type *llvmdestt = destt.toLLVMType(fix->getContext());
  
  Instruction *fixinst = dyn_cast<Instruction>(fix);
  if (!ip && fixinst) {
    ip = fixinst->getNextNode();
  }
  assert(ip && "ip required when converted value not an instruction");

  IRBuilder<> builder(ip);

  auto genSizeChange = [&](Value *fix) -> Value* {
    if (destt.isSigned) {
      return builder.CreateSExtOrTrunc(fix, llvmdestt);
    } else {
      return builder.CreateZExtOrTrunc(fix, llvmdestt);
    }
  };
  
  auto genPointMovement = [&](Value *fix) -> Value* {
    int deltab = destt.fracBitsAmt - srct.fracBitsAmt;
    if (deltab > 0) {
      return builder.CreateShl(fix, deltab);
    } else if (deltab < 0) {
      if (srct.isSigned) {
        return builder.CreateAShr(fix, -deltab);
      } else {
        return builder.CreateLShr(fix, -deltab);
      }
    }
    return fix;
  };
  
  if (destt.bitsAmt > srct.bitsAmt)
    return genPointMovement(genSizeChange(fix));
  return genSizeChange(genPointMovement(fix));
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, const FixedPointType& fixpt, Type *destt)
{
  dbgs() << "******** trace: genConvertFixToFloat ";
  fix->print(dbgs());
  dbgs() << " -> ";
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
  
  Value *floattmp = fixpt.isSigned ? builder.CreateSIToFP(fix, destt) : builder.CreateUIToFP(fix, destt);
  double twoebits = pow(2.0, fixpt.fracBitsAmt);
  return builder.CreateFDiv(floattmp, ConstantFP::get(destt, twoebits));
}


Type *FloatToFixed::getLLVMFixedPointTypeForFloatType(Type *srct, const FixedPointType& baset, bool *hasfloats)
{
  if (srct->isPointerTy()) {
    Type *enc = getLLVMFixedPointTypeForFloatType(srct->getPointerElementType(), baset, hasfloats);
    if (enc)
      return enc->getPointerTo();
    return nullptr;
    
  } else if (srct->isArrayTy()) {
    int nel = srct->getArrayNumElements();
    Type *enc = getLLVMFixedPointTypeForFloatType(srct->getArrayElementType(), baset, hasfloats);
    if (enc)
      return ArrayType::get(enc, nel);
    return nullptr;
    
  } else if (srct->isFloatingPointTy()) {
    if (hasfloats)
      *hasfloats = true;
    return baset.toLLVMType(srct->getContext());
    
  }
  DEBUG(
    dbgs() << "getLLVMFixedPointTypeForFloatType given non-float type ";
    srct->print(dbgs());
    dbgs() << "\n";
  );
  if (hasfloats)
    *hasfloats = false;
  return srct;
}


Type *FloatToFixed::getLLVMFixedPointTypeForFloatValue(Value *val)
{
  FixedPointType& fpt = fixPType(val);
  return getLLVMFixedPointTypeForFloatType(val->getType(), fpt);
}



