#include <sstream>
#include "llvm/Support/Debug.h"
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
#include "FloatTypeOverlay.h"
#include "FixedPointTypeOverlay.h"

using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


FloatTypeOverlay *FloatTypeOverlay::get(FloatToFixed *C, Type::TypeID typeId)
{
  auto existing = C->FPTypes.find(typeId);
  if (existing != C->FPTypes.end()) {
    return existing->getSecond();
  }
  
  if (!(typeId == Type::TypeID::HalfTyID || typeId == Type::TypeID::FloatTyID ||
    typeId == Type::TypeID::DoubleTyID ||
    typeId == Type::TypeID::X86_FP80TyID || typeId == Type::TypeID::FP128TyID ||
    typeId == Type::TypeID::PPC_FP128TyID)) {
    LLVM_DEBUG(dbgs() << "attempted to create a FloatTypeOverlay from non-float type ID " << typeId << "\n");
    return nullptr;
  }
  FloatTypeOverlay *newT = new FloatTypeOverlay(C, typeId);
  C->FPTypes[typeId] = newT;
  return newT;
}


std::string FloatTypeOverlay::toString() const
{
  switch (typeId) {
    case Type::TypeID::HalfTyID:
      return "half";
    case Type::TypeID::FloatTyID:
      return "float";
    case Type::TypeID::DoubleTyID:
      return "double";
    case Type::TypeID::X86_FP80TyID:
      return "x86fp80";
    case Type::TypeID::FP128TyID:
      return "float128";
    case Type::TypeID::PPC_FP128TyID:
      return "ppcfloat128";
    default:
      llvm_unreachable("FloatTypeOverlay type ID not a float!?");
  }
  return "ERROR";
}


Type *FloatTypeOverlay::getBaseLLVMType(LLVMContext& ctxt) const
{
  return Type::getPrimitiveType(ctxt, typeId);
}


Value *FloatTypeOverlay::genCastFrom(Value *I, const UniformTypeOverlay *IType, Instruction *IInsertBefore) const
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


Value *FloatTypeOverlay::genCastFrom(Value *fix, const FixedPointTypeOverlay *fixpt, Instruction *IInsertBefore) const
{
  LLVM_DEBUG(dbgs() << "******** trace: genConvertFixToFloat " << *fix << " -> " << *this << "\n");
  
  if (!fix->getType()->isIntegerTy()) {
    LLVM_DEBUG(errs() << "can't wrap-convert to flt non integer value " << *fix << "\n");
    return nullptr;
  }
  
  FixToFloatCount++;
  FixToFloatWeight += std::pow(2, std::min((int)(sizeof(int)*8-1), Context->getLoopNestingLevelOfValue(fix)));
  
  Type *destt = getBaseLLVMType(fix->getContext());
  
  if (Constant *cst = dyn_cast<Constant>(fix)) {
    Constant *floattmp = fixpt->getSigned() ?
      ConstantExpr::getSIToFP(cst, destt) :
      ConstantExpr::getUIToFP(cst, destt);
    double twoebits = pow(2.0, fixpt->getPointPos());
    return ConstantExpr::getFDiv(floattmp, ConstantFP::get(destt, twoebits));
  }
  
  IRBuilder<> builder(IInsertBefore);
  
  Value *floattmp = fixpt->getSigned() ? builder.CreateSIToFP(fix, destt) : builder.CreateUIToFP(fix, destt);
  Context->cpMetaData(floattmp,fix);
  double twoebits = pow(2.0, fixpt->getPointPos());
  return Context->cpMetaData(builder.CreateFDiv(floattmp,
                                       Context->cpMetaData(ConstantFP::get(destt, twoebits), fix)),fix);
}


Value *FloatTypeOverlay::genCastFrom(Value *I, const FloatTypeOverlay *IType, Instruction *IInsertBefore) const
{
  assert(I->getType()->getTypeID() == IType->typeId && "value has type incompatible with its type overlay");
  if (this == IType)
    return I;
    
  IRBuilder<> builder(IInsertBefore);
  Type *DestTy = getBaseLLVMType(I->getContext());
  return builder.CreateFPCast(I, DestTy);
}

