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
  
  inline llvm::Type::TypeID getTypeId() { return typeId; }
  
  using UniformTypeOverlay::genCastFrom;
  llvm::Value *genCastFrom(llvm::Value *I, UniformTypeOverlay *IType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genCastFrom(llvm::Value *I, FixedPointTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
  llvm::Value *genCastFrom(llvm::Value *I, FloatTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
};

}


#endif


