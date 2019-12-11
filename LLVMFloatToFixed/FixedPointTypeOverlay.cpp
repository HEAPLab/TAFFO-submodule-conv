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


Value *FixedPointTypeOverlay::genCastFrom(Value *I, const UniformTypeOverlay *IType, Instruction *IInsertBefore) const
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


Value *FixedPointTypeOverlay::genCastFrom(Value *fix, const FixedPointTypeOverlay *srct, Instruction *IInsertBefore) const
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


Value *FixedPointTypeOverlay::genCastFrom(Value *flt, const FloatTypeOverlay *IType, Instruction *ip) const
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


Value *FixedPointTypeOverlay::genCastOnlyIfNotFixedPoint(Value *I, const UniformTypeOverlay *IType, const FixedPointTypeOverlay **OutIType, Instruction *IInsertBefore) const
{
  const FixedPointTypeOverlay *IFXType = dyn_cast<FixedPointTypeOverlay>(IType);
  if (!IFXType) {
    if (OutIType) *OutIType = this;
    return genCastFrom(I, IType, IInsertBefore);
  }
  if (OutIType) *OutIType = IFXType;
  return I;
}


Value *FixedPointTypeOverlay::genAdd(Value *A, const UniformTypeOverlay *AType, Value *B, const UniformTypeOverlay *BType, Instruction *IInsertBefore) const
{
  Value *val1 = genCastFrom(A, AType, IInsertBefore);
  Value *val2 = genCastFrom(B, BType, IInsertBefore);
  assert(val1 && val2);
  
  IRBuilder<> builder(IInsertBefore);
  return builder.CreateBinOp(Instruction::Add, val1, val2);
}


Value *FixedPointTypeOverlay::genSub(Value *A, const UniformTypeOverlay *AType, Value *B, const UniformTypeOverlay *BType, Instruction *IInsertBefore) const
{
  // TODO: improve overflow resistance by shifting late
  Value *val1 = genCastFrom(A, AType, IInsertBefore);
  Value *val2 = genCastFrom(B, BType, IInsertBefore);
  assert(val1 && val2);
  
  IRBuilder<> builder(IInsertBefore);
  return builder.CreateBinOp(Instruction::Sub, val1, val2);
}


Value *FixedPointTypeOverlay::genRem(Value *A, const UniformTypeOverlay *AType, Value *B, const UniformTypeOverlay *BType, Instruction *IInsertBefore) const
{
  Value *val1 = genCastFrom(A, AType, IInsertBefore);
  Value *val2 = genCastFrom(B, BType, IInsertBefore);
  assert(val1 && val2);
  
  IRBuilder<> builder(IInsertBefore);
  if (getSigned())
    return builder.CreateBinOp(Instruction::SRem, val1, val2);
  else
    return builder.CreateBinOp(Instruction::URem, val1, val2);
}


Value *FixedPointTypeOverlay::genMul(Value *A, const UniformTypeOverlay *AType, Value *B, const UniformTypeOverlay *BType, Instruction *IInsertBefore) const
{
  const FixedPointTypeOverlay *AFXType, *BFXType;
  // TODO: better default type cast here
  A = genCastOnlyIfNotFixedPoint(A, AType, &AFXType, IInsertBefore);
  B = genCastOnlyIfNotFixedPoint(B, BType, &BFXType, IInsertBefore);
  assert(A && B);
  
  FixedPointTypeOverlay *intermtype = FixedPointTypeOverlay::get(Context,
    getSigned(),
    AFXType->getPointPos() + BFXType->getPointPos(),
    AFXType->getSize() + BFXType->getSize());
  Type *dbfxt = intermtype->getBaseLLVMType(IInsertBefore->getContext());
  
  IRBuilder<> builder(IInsertBefore);
  
  Value *ext1 = AFXType->getSigned() ? builder.CreateSExt(A, dbfxt) : builder.CreateZExt(A, dbfxt);
  Context->cpMetaData(ext1, A);
  Value *ext2 = BFXType->getSigned() ? builder.CreateSExt(B, dbfxt) : builder.CreateZExt(B, dbfxt);
  Context->cpMetaData(ext2, B);
  Value *fixop = builder.CreateMul(ext1, ext2);
  /* FIXME
  cpMetaData(fixop,instr);
  updateFPTypeMetadata(fixop, intermtype->getSigned(), intermtype->getPointPos(), intermtype->getSize());
  updateConstTypeMetadata(fixop, 0U, intype1f);
  updateConstTypeMetadata(fixop, 1U, intype2f);
  */
  return genCastFrom(fixop, intermtype, IInsertBefore);
}


Value *FixedPointTypeOverlay::genDiv(Value *A, const UniformTypeOverlay *AType, Value *B, const UniformTypeOverlay *BType, Instruction *IInsertBefore) const
{
  const FixedPointTypeOverlay *AFXType, *BFXType;
  // TODO: better default type cast here
  A = genCastOnlyIfNotFixedPoint(A, AType, &AFXType, IInsertBefore);
  B = genCastOnlyIfNotFixedPoint(B, BType, &BFXType, IInsertBefore);
  assert(A && B);

  FixedPointTypeOverlay *intermtype = FixedPointTypeOverlay::get(Context,
    getSigned(),
    AFXType->getPointPos() + BFXType->getPointPos(),
    AFXType->getSize() + BFXType->getSize());
  Type *dbfxt = intermtype->getBaseLLVMType(IInsertBefore->getContext());
  
  FixedPointTypeOverlay *fixoptype = FixedPointTypeOverlay::get(Context,
    getSigned(),
    AFXType->getPointPos(),
    AFXType->getSize() + BFXType->getSize());
  Value *ext1 = intermtype->genCastFrom(A, AFXType, IInsertBefore);
  IRBuilder<> builder(IInsertBefore);
  Value *ext2 = BFXType->getSigned() ? builder.CreateSExt(B, dbfxt) : builder.CreateZExt(B, dbfxt);
  Value *fixop = getSigned() ? builder.CreateSDiv(ext1, ext2) : builder.CreateUDiv(ext1, ext2);
  /* FIXME
  cpMetaData(ext1,val1);
  cpMetaData(ext2,val2);
  cpMetaData(fixop,instr);
  updateFPTypeMetadata(fixop, fixoptype->getSigned(), fixoptype->getPointPos(), fixoptype->getSize());
  updateConstTypeMetadata(fixop, 0U, intermtype);
  updateConstTypeMetadata(fixop, 1U, intype2f);
  */
  return genCastFrom(fixop, fixoptype, IInsertBefore);
}


Value *FixedPointTypeOverlay::genCmp(CmpInst::Predicate Pred, Value *FXIThis, Value *IOther, const UniformTypeOverlay *OtherType, Instruction *IInsertBefore) const
{
  const FixedPointTypeOverlay *OtherFXType;
  Value *FXIOther = genCastOnlyIfNotFixedPoint(IOther, OtherType, &OtherFXType, IInsertBefore);
  
  bool mixedsign = getSigned() != OtherFXType->getSigned();
  int intpart1 = getSize() - getPointPos() + (mixedsign ? getSigned() : 0);
  int intpart2 = OtherFXType->getSize() - OtherFXType->getPointPos() + (mixedsign ? OtherFXType->getSigned() : 0);
  
  int pointPos = std::max(getPointPos(), OtherFXType->getPointPos());
  FixedPointTypeOverlay *cmptype = FixedPointTypeOverlay::get(Context,
    getSigned() || OtherFXType->getSigned(),
    pointPos,
    std::max(intpart1, intpart2) + pointPos);
  
  Value *AdjFXIThis = cmptype->genCastFrom(FXIThis, this, IInsertBefore);
  Value *AdjFXIOther = cmptype->genCastFrom(FXIOther, OtherFXType, IInsertBefore);
  
  IRBuilder<> builder(IInsertBefore);
  CmpInst::Predicate ty;
  int pr = Pred;
  bool swapped = false;

  //se unordered swappo, converto con la int, e poi mi ricordo di riswappare
  if (!CmpInst::isOrdered(Pred)) {
    pr = CmpInst::getInversePredicate(Pred);
    swapped = true;
  }

  if (pr == CmpInst::FCMP_OEQ){
    ty = CmpInst::ICMP_EQ;
  } else if (pr == CmpInst::FCMP_ONE) {
    ty = CmpInst::ICMP_NE;
  } else if (pr == CmpInst::FCMP_OGT) {
    ty = cmptype->getSigned() ? CmpInst::ICMP_SGT : CmpInst::ICMP_UGT;
  } else if (pr == CmpInst::FCMP_OGE) {
    ty = cmptype->getSigned() ? CmpInst::ICMP_SGE : CmpInst::ICMP_UGE;
  } else if (pr == CmpInst::FCMP_OLE) {
    ty = cmptype->getSigned() ? CmpInst::ICMP_SLE : CmpInst::ICMP_ULE;
  } else if (pr == CmpInst::FCMP_OLT) {
    ty = cmptype->getSigned() ? CmpInst::ICMP_SLT : CmpInst::ICMP_ULT;
  } else if (pr == CmpInst::FCMP_ORD) {
    ;
    //TODO gestione NaN
  } else if (pr == CmpInst::FCMP_TRUE) {
    /* there is no integer-only always-true / always-false comparison
     * operator... so we roll out our own by producing a tautology */
    return builder.CreateICmpEQ(
      ConstantInt::get(Type::getInt32Ty(IInsertBefore->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(IInsertBefore->getContext()),0));
  } else if (pr == CmpInst::FCMP_FALSE) {
    return builder.CreateICmpNE(
      ConstantInt::get(Type::getInt32Ty(IInsertBefore->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(IInsertBefore->getContext()),0));
  }

  if (swapped) {
    ty = CmpInst::getInversePredicate(ty);
  }

  return builder.CreateICmp(ty, AdjFXIThis, AdjFXIOther);
}

