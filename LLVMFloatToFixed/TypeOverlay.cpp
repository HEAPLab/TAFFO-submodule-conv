#include <sstream>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "TypeUtils.h"
#include "TypeOverlay.h"

using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


VoidTypeOverlay VoidTypeOverlay::Void;
llvm::DenseMap<uint64_t, FixedPointTypeOverlay *> FixedPointTypeOverlay::FXTypes;
llvm::StringMap<StructTypeOverlay *> StructTypeOverlay::StructTypes;


TypeOverlay *TypeOverlay::get(llvm::Type *llvmtype, bool signd)
{
  if (llvmtype->isIntegerTy()) {
    return FixedPointTypeOverlay::get(signd, 0, llvmtype->getIntegerBitWidth());
  }
  return VoidTypeOverlay::get();
}


TypeOverlay *TypeOverlay::get(mdutils::MDInfo *mdnfo, int *enableConversion)
{
  if (!mdnfo) {
    return VoidTypeOverlay::get();
    
  } else if (InputInfo *ii = dyn_cast<InputInfo>(mdnfo)) {
    if (ii->IEnableConversion) {
      if (enableConversion) (*enableConversion)++;
      return TypeOverlay::get(ii->IType.get());
    } else {
      return VoidTypeOverlay::get();
    }
    
  } else if (StructInfo *si = dyn_cast<StructInfo>(mdnfo)) {
    SmallVector<TypeOverlay *, 2> elems;
    for (auto i = si->begin(); i != si->end(); i++) {
      elems.push_back(TypeOverlay::get(i->get(), enableConversion));
    }
    return StructTypeOverlay::get(elems);
    
  }
  
  assert(false && "BUG: unknown type of MDInfo");
  return VoidTypeOverlay::get();
}


TypeOverlay *TypeOverlay::get(mdutils::TType *mdtype)
{
  if (!mdtype) {
    return VoidTypeOverlay::get();
  } else if (FPType *fpt = dyn_cast_or_null<FPType>(mdtype)) {
    return FixedPointTypeOverlay::get(fpt->isSigned(), fpt->getPointPos(), fpt->getWidth());
  }
  
  assert(false && "BUG: unknown TType");
  return VoidTypeOverlay::get();
}


TypeOverlay *TypeOverlay::unwrapIndexList(llvm::Type *valType, const llvm::iterator_range<const llvm::Use*> indices)
{
  Type *resolvedType = valType;
  TypeOverlay *tempFixpt = this;
  for (Value *a : indices) {
    if (resolvedType->isPointerTy()) {
      resolvedType = resolvedType->getPointerElementType();
    } else if (resolvedType->isArrayTy()) {
      resolvedType = resolvedType->getArrayElementType();
    } else if (resolvedType->isVectorTy()) {
      resolvedType = resolvedType->getVectorElementType();
    } else if (resolvedType->isStructTy()) {
      ConstantInt *val = dyn_cast<ConstantInt>(a);
      assert(val && "non-constant index for struct in GEP");
      int n = val->getZExtValue();
      resolvedType = resolvedType->getStructElementType(n);
      StructTypeOverlay *TOStruct = dyn_cast<StructTypeOverlay>(tempFixpt);
      assert(TOStruct && "non-conforming type and index list");
      tempFixpt = TOStruct->item(n);
    } else {
      assert("unsupported type in GEP");
    }
  }
  return tempFixpt;
}


TypeOverlay *TypeOverlay::unwrapIndexList(llvm::Type *valType, llvm::ArrayRef<unsigned> indices)
{
  Type *resolvedType = valType;
  TypeOverlay *tempFixpt = this;
  for (unsigned n : indices) {
    if (resolvedType->isPointerTy()) {
      resolvedType = resolvedType->getPointerElementType();
    } else if (resolvedType->isArrayTy()) {
      resolvedType = resolvedType->getArrayElementType();
    } else if (resolvedType->isVectorTy()) {
      resolvedType = resolvedType->getVectorElementType();
    } else if (resolvedType->isStructTy()) {
      resolvedType = resolvedType->getStructElementType(n);
      StructTypeOverlay *TOStruct = dyn_cast<StructTypeOverlay>(tempFixpt);
      assert(TOStruct && "non-conforming type and index list");
      tempFixpt = TOStruct->item(n);
    } else {
      assert("unsupported type in GEP");
    }
  }
  return tempFixpt;
}


raw_ostream& operator<<(raw_ostream& stm, const TypeOverlay& f)
{
  stm << f.toString();
  return stm;
}


FixedPointTypeOverlay *FixedPointTypeOverlay::get(bool s, int f, int b)
{
  assert(std::abs(f) < (1 << 28) && std::abs(b) < (1 << 28) && "exceeded max size = 268435455 bits");
  assert(b > 0 && "total number of bits in representation must be > 0");
  uint64_t key =
    (((uint64_t)f & 0xFFFFFFF) << (28+1)) +
    (((uint64_t)b & 0xFFFFFFF) << (1)) +
    (s & 0x1);
  auto match = FixedPointTypeOverlay::FXTypes.find(key);
  if (match != FixedPointTypeOverlay::FXTypes.end())
    return match->getSecond();
  
  FixedPointTypeOverlay *res = new FixedPointTypeOverlay(s, f, b);
  FixedPointTypeOverlay::FXTypes[key] = res;
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


Type *FixedPointTypeOverlay::scalarToLLVMType(LLVMContext& ctxt) const
{
  return Type::getIntNTy(ctxt, bitsAmt);
}


StructTypeOverlay::StructTypeOverlay(const ArrayRef<TypeOverlay *>& elems) : TypeOverlay(TOK_Struct)
{
  elements.insert(elements.begin(), elems.begin(), elems.end());
}


StructTypeOverlay *StructTypeOverlay::get(const llvm::ArrayRef<TypeOverlay *>& elems)
{
  StructTypeOverlay *res = new StructTypeOverlay(elems);
  std::string key = res->toString();
  
  auto match = StructTypeOverlay::StructTypes.find(key);
  if (match != StructTypeOverlay::StructTypes.end()) {
    delete res;
    return match->second;
  }
  
  StructTypeOverlay::StructTypes[key] = res;
  return res;
}


std::string StructTypeOverlay::toString() const
{
  std::stringstream stm;
  
  stm << '<';
  for (int i=0; i<elements.size(); i++) {
    stm << elements[i]->toString();
    if (i != elements.size()-1)
      stm << ',';
  }
  stm << '>';
  
  return stm.str();
}


bool StructTypeOverlay::isRecursivelyVoid() const
{
  for (TypeOverlay *fpt: elements) {
    if (fpt->isRecursivelyVoid())
      return true;
  }
  return false;
}

