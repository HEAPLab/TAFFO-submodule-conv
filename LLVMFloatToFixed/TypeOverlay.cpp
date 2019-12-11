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
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"
#include "TypeOverlay.h"
#include "FixedPointTypeOverlay.h"

using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


TypeOverlay *TypeOverlay::get(FloatToFixed *C, llvm::Type *llvmtype, bool signd)
{
  if (llvmtype->isIntegerTy()) {
    return FixedPointTypeOverlay::get(C, signd, 0, llvmtype->getIntegerBitWidth());
  }
  return VoidTypeOverlay::get(C);
}


TypeOverlay *TypeOverlay::get(FloatToFixed *C, mdutils::MDInfo *mdnfo, int *enableConversion)
{
  if (!mdnfo) {
    return VoidTypeOverlay::get(C);
    
  } else if (InputInfo *ii = dyn_cast<InputInfo>(mdnfo)) {
    if (ii->IEnableConversion) {
      if (enableConversion) (*enableConversion)++;
      return TypeOverlay::get(C, ii->IType.get());
    } else {
      return VoidTypeOverlay::get(C);
    }
    
  } else if (StructInfo *si = dyn_cast<StructInfo>(mdnfo)) {
    SmallVector<TypeOverlay *, 2> elems;
    for (auto i = si->begin(); i != si->end(); i++) {
      elems.push_back(TypeOverlay::get(C, i->get(), enableConversion));
    }
    return StructTypeOverlay::get(C, elems);
    
  }
  
  assert(false && "BUG: unknown type of MDInfo");
  return VoidTypeOverlay::get(C);
}


TypeOverlay *TypeOverlay::get(FloatToFixed *C, mdutils::TType *mdtype)
{
  if (!mdtype) {
    return VoidTypeOverlay::get(C);
  } else if (FPType *fpt = dyn_cast_or_null<FPType>(mdtype)) {
    return FixedPointTypeOverlay::get(C, fpt->isSigned(), fpt->getPointPos(), fpt->getWidth());
  }
  
  assert(false && "BUG: unknown TType");
  return VoidTypeOverlay::get(C);
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


Type *TypeOverlay::overlayOnBaseType(Type *srct, bool *changed) const
{
  if (srct->isPointerTy()) {
    Type *enc = overlayOnBaseType(srct->getPointerElementType(), changed);
    if (enc)
      return enc->getPointerTo();
    return nullptr;
    
  } else if (srct->isArrayTy()) {
    int nel = srct->getArrayNumElements();
    Type *enc = overlayOnBaseType(srct->getArrayElementType(), changed);
    if (enc)
      return ArrayType::get(enc, nel);
    return nullptr;
    
  } else if (srct->isStructTy()) {
    SmallVector<Type *, 2> elems;
    bool allinvalid = true;
    for (int i=0; i<srct->getStructNumElements(); i++) {
      const StructTypeOverlay *structt = cast<StructTypeOverlay>(this);
      TypeOverlay *fpelemt = structt->item(i);
      Type *baseelemt = srct->getStructElementType(i);
      Type *newelemt;
      if (!fpelemt->isVoid()) {
        allinvalid = false;
        newelemt = fpelemt->overlayOnBaseType(baseelemt, changed);
      } else {
        newelemt = baseelemt;
      }
      elems.push_back(newelemt);
    }
    if (!allinvalid)
      return StructType::get(srct->getContext(), elems, dyn_cast<StructType>(srct)->isPacked());
    return srct;
    
  } else if (srct->isFloatingPointTy() || srct->isIntegerTy()) {
    if (!this->isVoid()) {
      if (changed)
        *changed = true;
      return cast<UniformTypeOverlay>(this)->getBaseLLVMType(srct->getContext());
    }
    return srct;
    
  }
  LLVM_DEBUG(dbgs() << "getLLVMFixedPointTypeForFloatType given unexpected type " << *srct << "\n");
  if (changed)
    *changed = false;
  return srct;
}


raw_ostream& operator<<(raw_ostream& stm, const TypeOverlay& f)
{
  stm << f.toString();
  return stm;
}


VoidTypeOverlay *VoidTypeOverlay::get(FloatToFixed *C)
{
  if (C->Void == nullptr)
    C->Void = new VoidTypeOverlay(C);
  return C->Void;
}


StructTypeOverlay::StructTypeOverlay(FloatToFixed *C, const ArrayRef<TypeOverlay *>& elems) : TypeOverlay(TOK_Struct, C)
{
  elements.insert(elements.begin(), elems.begin(), elems.end());
}


StructTypeOverlay *StructTypeOverlay::get(FloatToFixed *C, const llvm::ArrayRef<TypeOverlay *>& elems)
{
  StructTypeOverlay *res = new StructTypeOverlay(C, elems);
  std::string key = res->toString();
  
  auto match = C->StructTypes.find(key);
  if (match != C->StructTypes.end()) {
    delete res;
    return match->second;
  }
  
  C->StructTypes[key] = res;
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


Value *UniformTypeOverlay::genCastFrom(llvm::Value *I, const UniformTypeOverlay *IType) const
{
  if (Instruction *i = dyn_cast<Instruction>(I))
    return genCastFrom(i, IType);
  else if (Argument *v = dyn_cast<Argument>(I))
    return genCastFrom(v, IType);
  assert(false && "genCastFrom: value not an instruction or an argument");
}


Value *UniformTypeOverlay::genCastFrom(Instruction *I, const UniformTypeOverlay *IType) const
{
  Instruction *ip = getFirstInsertionPointAfter(I);
  return genCastFrom(I, IType, ip);
};


Value *UniformTypeOverlay::genCastFrom(Argument *arg, const UniformTypeOverlay *IType) const
{
  Function *fun = arg->getParent();
  BasicBlock& firstbb = fun->getEntryBlock();
  Instruction *ip = &(*firstbb.getFirstInsertionPt());
  return genCastFrom(arg, IType, ip);
}



