#include <sstream>
#include "llvm/Support/Debug.h"
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"
#include "FloatTypeOverlay.h"

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

