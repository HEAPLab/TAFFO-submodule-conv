#include "TypeOverlay.h"

#ifndef __FLOAT_TYPE_OVERLAY_H__
#define __FLOAT_TYPE_OVERLAY_H__


namespace flttofix {

class FloatTypeOverlay : public UniformTypeOverlay {
  llvm::Type::TypeID typeId;
  
  static llvm::DenseMap<int, FloatTypeOverlay *> FPTypes;
  
protected:
  FloatTypeOverlay(llvm::Type::TypeID TypeId): UniformTypeOverlay(TOK_FixedPoint), typeId(TypeId) {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Float; }
  static FloatTypeOverlay *get(llvm::Type::TypeID typeId);
  static FloatTypeOverlay *get(llvm::Type *llvmtype)
  {
    return FloatTypeOverlay::get(llvmtype->getTypeID());
  }
  
  std::string toString() const override;
  llvm::Type *getBaseLLVMType(llvm::LLVMContext& ctxt) const override;
  
  inline llvm::Type::TypeID getTypeId() { return typeId; }
};

}


#endif


