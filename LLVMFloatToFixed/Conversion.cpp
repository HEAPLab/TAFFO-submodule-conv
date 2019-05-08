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
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"

using namespace llvm;
using namespace flttofix;
using namespace taffo;


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
    
    LLVM_DEBUG(dbgs() << "performConversion " << *v << "\n");
    
    Value *newv = convertSingleValue(m, v, valueInfo(v)->fixpType);
    if (newv) {
      operandPool[v] = newv;
    }
    
    if (newv && newv != ConversionError) {
      if (newv != v && isa<Instruction>(newv) && isa<Instruction>(v)) {
        Instruction *newinst = dyn_cast<Instruction>(newv);
        Instruction *oldinst = dyn_cast<Instruction>(v);
        newinst->setDebugLoc(oldinst->getDebugLoc());
      }
      cpMetaData(newv,v);
      *valueInfo(newv) = *valueInfo(v);
    } else {
      LLVM_DEBUG(dbgs() << "warning: ";
            v->print(dbgs());
            dbgs() << " not converted\n";);
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
  } else if (Argument *argument = dyn_cast<Argument>(val)) {
    if (isFloatType(argument->getType()))
      res = translateOrMatchOperand(val, fixpt, nullptr);
    else
      res = val;
  }
  
  return res ? res : ConversionError;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(Value *val, FixedPointType& iofixpt, Instruction *ip)
{
  assert(val->getType()->getNumContainedTypes() == 0 && "translateOrMatchOperand val is not a scalar value");
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

  assert(val->getType()->isFloatingPointTy());
  return genConvertFloatToFix(val, iofixpt, ip);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt, const FixedPointType& fixpt, Instruction *ip)
{
  assert(flt->getType()->isFloatingPointTy() && "genConvertFloatToFixed called on a non-float scalar");
  
  if (Constant *c = dyn_cast<Constant>(flt)) {
    FixedPointType fixptcopy = fixpt;
    Value *res = convertConstant(c, fixptcopy);
    assert(fixptcopy == fixpt && "why is there a pointer here?");
    return res;
  }

  if (Instruction *i = dyn_cast<Instruction>(flt)) {
    Instruction *tentativeIp = i->getNextNode();
    if (tentativeIp)
      ip = tentativeIp;
    else
      LLVM_DEBUG(dbgs() << "warning: genConvertFloatToFix on a BB-terminating inst\n");
  } else if (Argument *arg = dyn_cast<Argument>(flt)) {
    Function *fun = arg->getParent();
    BasicBlock& firstbb = fun->getEntryBlock();
    ip = &(*firstbb.getFirstInsertionPt());
  }
  assert(ip && "ip is mandatory if not passing an instruction/constant value");
  
  FloatToFixCount++;
  FloatToFixWeight += std::pow(2, std::min((int)(sizeof(int)*8-1), this->getLoopNestingLevelOfValue(flt)));
  
  IRBuilder<> builder(ip);
  Type *destt = getLLVMFixedPointTypeForFloatType(flt->getType(), fixpt);
  
  /* insert new instructions before ip */
  if (SIToFPInst *instr = dyn_cast<SIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return cpMetaData(builder.CreateShl(
              cpMetaData(builder.CreateIntCast(intparam, destt, true),flt,ip),
            fixpt.scalarFracBitsAmt()),flt,ip);
  } else if (UIToFPInst *instr = dyn_cast<UIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return cpMetaData(builder.CreateShl(
              cpMetaData(builder.CreateIntCast(intparam, destt, false),flt,ip),
            fixpt.scalarFracBitsAmt()),flt,ip);
  } else {
    double twoebits = pow(2.0, fixpt.scalarFracBitsAmt());
    Value *interm = cpMetaData(builder.CreateFMul(
          cpMetaData(ConstantFP::get(flt->getType(), twoebits),flt,ip),
        flt),flt,ip);
    if (fixpt.scalarIsSigned()) {
      return cpMetaData(builder.CreateFPToSI(interm, destt),flt,ip);
    } else {
      return cpMetaData(builder.CreateFPToUI(interm, destt),flt,ip);
    }
  }
}


Value *FloatToFixed::genConvertFixedToFixed(Value *fix, const FixedPointType& srct, const FixedPointType& destt, Instruction *ip)
{
  if (srct == destt)
    return fix;
  
  Type *llvmsrct = fix->getType();
  assert(llvmsrct->isSingleValueType() && "cannot change fixed point format of a pointer");
  assert(llvmsrct->isIntegerTy() && "cannot change fixed point format of a float");
  
  Type *llvmdestt = destt.scalarToLLVMType(fix->getContext());
  
  Instruction *fixinst = dyn_cast<Instruction>(fix);
  if (fixinst)
    ip = fixinst->getNextNode();
  assert(ip && "ip required when converted value not an instruction");

  IRBuilder<> builder(ip);

  auto genSizeChange = [&](Value *fix) -> Value* {
    if (destt.scalarIsSigned()) {
      return cpMetaData(builder.CreateSExtOrTrunc(fix, llvmdestt),fix);
    } else {
      return cpMetaData(builder.CreateZExtOrTrunc(fix, llvmdestt),fix);
    }
  };
  
  auto genPointMovement = [&](Value *fix) -> Value* {
    int deltab = destt.scalarFracBitsAmt() - srct.scalarFracBitsAmt();
    if (deltab > 0) {
      return cpMetaData(builder.CreateShl(fix, deltab),fix);
    } else if (deltab < 0) {
      if (srct.scalarIsSigned()) {
        return cpMetaData(builder.CreateAShr(fix, -deltab),fix);
      } else {
        return cpMetaData(builder.CreateLShr(fix, -deltab),fix);
      }
    }
    return fix;
  };
  
  if (destt.scalarBitsAmt() > srct.scalarBitsAmt())
    return genPointMovement(genSizeChange(fix));
  return genSizeChange(genPointMovement(fix));
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, const FixedPointType& fixpt, Type *destt)
{
  LLVM_DEBUG(dbgs() << "******** trace: genConvertFixToFloat ";
  fix->print(dbgs());
  dbgs() << " -> ";
  destt->print(dbgs());
  dbgs() << "\n";);
  
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;
  FixToFloatCount++;
  FixToFloatWeight += std::pow(2, std::min((int)(sizeof(int)*8-1), this->getLoopNestingLevelOfValue(fix)));
  
  if (!fix->getType()->isIntegerTy()) {
    LLVM_DEBUG(errs() << "can't wrap-convert to flt non integer value ";
          fix->print(errs());
          errs() << "\n");
    return nullptr;
  }

  IRBuilder<> builder(i->getNextNode());
  
  Value *floattmp = fixpt.scalarIsSigned() ? builder.CreateSIToFP(fix, destt) : builder.CreateUIToFP(fix, destt);
  cpMetaData(floattmp,fix);
  double twoebits = pow(2.0, fixpt.scalarFracBitsAmt());
  return cpMetaData(builder.CreateFDiv(floattmp,
                                       cpMetaData(ConstantFP::get(destt, twoebits), fix)),fix);
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
    
  } else if (srct->isStructTy()) {
    SmallVector<Type *, 2> elems;
    bool allinvalid = true;
    for (int i=0; i<srct->getStructNumElements(); i++) {
      const FixedPointType& fpelemt = baset.structItem(i);
      Type *baseelemt = srct->getStructElementType(i);
      Type *newelemt;
      if (!fpelemt.isInvalid()) {
        allinvalid = false;
        newelemt = getLLVMFixedPointTypeForFloatType(baseelemt, fpelemt, hasfloats);
      } else {
        newelemt = baseelemt;
      }
      elems.push_back(newelemt);
    }
    if (!allinvalid)
      return StructType::get(srct->getContext(), elems, dyn_cast<StructType>(srct)->isPacked());
    return srct;
    
  } else if (srct->isFloatingPointTy()) {
    if (hasfloats)
      *hasfloats = true;
    return baset.scalarToLLVMType(srct->getContext());
    
  }
  LLVM_DEBUG(
    dbgs() << "getLLVMFixedPointTypeForFloatType given unexpected non-float type ";
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



