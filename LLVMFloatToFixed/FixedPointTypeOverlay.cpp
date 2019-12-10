#include <sstream>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"
#include "FixedPointTypeOverlay.h"
#include "FloatTypeOverlay.h"

using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


FixedPointTypeOverlay *FixedPointTypeOverlay::get(FloatToFixed *C, bool s, int f, int b)
{
  assert(std::abs(f) < (1 << 28) && std::abs(b) < (1 << 28) && "exceeded max size = 268435455 bits");
  assert(b > 0 && "total number of bits in representation must be > 0");
  uint64_t key =
    (((uint64_t)f & 0xFFFFFFF) << (28+1)) +
    (((uint64_t)b & 0xFFFFFFF) << (1)) +
    (s & 0x1);
  auto match = C->FXTypes.find(key);
  if (match != C->FXTypes.end())
    return match->getSecond();
  
  FixedPointTypeOverlay *res = new FixedPointTypeOverlay(C,s, f, b);
  C->FXTypes[key] = res;
  return res;
}


std::string FixedPointTypeOverlay::toString() const
{
  std::stringstream stm;
  
  if (isSigned)
    stm << "s";
  else
    stm << "u";
  stm << bitsAmt - fracBitsAmt << "_" << fracBitsAmt << "fixp";
  
  return stm.str();
}


Type *FixedPointTypeOverlay::getBaseLLVMType(LLVMContext& ctxt) const
{
  return Type::getIntNTy(ctxt, bitsAmt);
}


Value *FixedPointTypeOverlay::genCastFrom(Value *I, UniformTypeOverlay *IType, Instruction *IInsertBefore) const
{
  switch (IType->getKind()) {
    case TypeOverlay::TOK_FixedPoint:
      return genCastFrom(I, cast<FixedPointTypeOverlay>(IType), IInsertBefore);
    case TypeOverlay::TOK_Float:
      return genCastFrom(I, cast<FloatTypeOverlay>(IType), IInsertBefore);
    default:
      llvm_unreachable("genCastFrom IType not an uniform");
  }
  return nullptr;
}


Value *FixedPointTypeOverlay::genCastFrom(Value *fix, FixedPointTypeOverlay *srct, Instruction *IInsertBefore) const
{
  if (srct == this)
    return fix;
  
  Type *llvmsrct = fix->getType();
  Type *llvmdestt = getBaseLLVMType(fix->getContext());
  assert(llvmsrct->isSingleValueType() && "cannot change fixed point format of a pointer");
  assert(llvmsrct->isIntegerTy() && "cannot change fixed point format of a float");
  
  IRBuilder<> builder(IInsertBefore);

  auto genSizeChange = [&](Value *fix) -> Value* {
    if (srct->getSigned()) {
      return Context->cpMetaData(builder.CreateSExtOrTrunc(fix, llvmdestt),fix);
    } else {
      return Context->cpMetaData(builder.CreateZExtOrTrunc(fix, llvmdestt),fix);
    }
  };
  
  auto genPointMovement = [&](Value *fix) -> Value* {
    int deltab = getPointPos() - srct->getPointPos();
    if (deltab > 0) {
      return Context->cpMetaData(builder.CreateShl(fix, deltab),fix);
    } else if (deltab < 0) {
      if (srct->getSigned()) {
        return Context->cpMetaData(builder.CreateAShr(fix, -deltab),fix);
      } else {
        return Context->cpMetaData(builder.CreateLShr(fix, -deltab),fix);
      }
    }
    return fix;
  };
  
  if (getSize() > srct->getSize())
    return genPointMovement(genSizeChange(fix));
  return genSizeChange(genPointMovement(fix));
}


Value *FixedPointTypeOverlay::genCastFrom(Value *flt, FloatTypeOverlay *IType, Instruction *ip) const
{
  assert(flt->getType()->isFloatingPointTy() && "genConvertFloatToFixed called on a non-float scalar");
  assert(flt->getType()->getTypeID() == IType->getTypeId() && "FloatTypeOverlay not coherent with value type");
  
  FloatToFixCount++;
  FloatToFixWeight += std::pow(2, std::min((int)(sizeof(int)*8-1), Context->getLoopNestingLevelOfValue(flt)));
  
  IRBuilder<> builder(ip);
  Type *destt = overlayOnBaseType(flt->getType());
  
  /* insert new instructions before ip */
  if (SIToFPInst *instr = dyn_cast<SIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return Context->cpMetaData(builder.CreateShl(
              Context->cpMetaData(builder.CreateIntCast(intparam, destt, true),flt,ip),
            getPointPos()),flt,ip);
            
  } else if (UIToFPInst *instr = dyn_cast<UIToFPInst>(flt)) {
    Value *intparam = instr->getOperand(0);
    return Context->cpMetaData(builder.CreateShl(
              Context->cpMetaData(builder.CreateIntCast(intparam, destt, false),flt,ip),
            getPointPos()),flt,ip);
            
  } else {
    double twoebits = pow(2.0, getPointPos());
    Value *interm = Context->cpMetaData(builder.CreateFMul(
          Context->cpMetaData(ConstantFP::get(flt->getType(), twoebits),flt,ip),
        flt),flt,ip);
    if (getSigned()) {
      return Context->cpMetaData(builder.CreateFPToSI(interm, destt),flt,ip);
    } else {
      return Context->cpMetaData(builder.CreateFPToUI(interm, destt),flt,ip);
    }
  }
}


