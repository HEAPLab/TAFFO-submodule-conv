#ifndef __FLOAT_TYPE_OVERLAY_H__
#define __FLOAT_TYPE_OVERLAY_H__

#include "TypeOverlay.h"


namespace flttofix {

class FixedPointTypeOverlay;

class FloatTypeOverlay : public UniformTypeOverlay {
  llvm::Type::TypeID typeId;
  
protected:
  FloatTypeOverlay(FloatToFixed *C, llvm::Type::TypeID TypeId): UniformTypeOverlay(TOK_Float, C), typeId(TypeId) {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Float; }
  static FloatTypeOverlay *get(FloatToFixed *C, llvm::Type::TypeID typeId);
  static FloatTypeOverlay *get(FloatToFixed *C, llvm::Type *llvmtype)
  {
    return FloatTypeOverlay::get(C, llvmtype->getTypeID());
  }
  
  std::string toString() const override;
  llvm::Type *getBaseLLVMType(llvm::LLVMContext& ctxt) const override;
  
  inline llvm::Type::TypeID getTypeId() const { return typeId; }
  
  using UniformTypeOverlay::genCastFrom;
  llvm::Value *genCastFrom(llvm::Value *I, const UniformTypeOverlay *IType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genCastFrom(llvm::Value *I, const FixedPointTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
  llvm::Value *genCastFrom(llvm::Value *I, const FloatTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
};

}


#endif


