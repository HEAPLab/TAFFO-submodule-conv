#include "TypeOverlay.h"

#ifndef __FIXED_POINT_TYPE_OVERLAY_H__
#define __FIXED_POINT_TYPE_OVERLAY_H__


namespace flttofix {

class FixedPointTypeOverlay : public UniformTypeOverlay {
  bool isSigned;
  int fracBitsAmt;
  int bitsAmt;
  
  static llvm::DenseMap<uint64_t, FixedPointTypeOverlay *> FXTypes;
  
protected:
  FixedPointTypeOverlay(bool s, int f, int b): UniformTypeOverlay(TOK_FixedPoint), isSigned(s), fracBitsAmt(f), bitsAmt(b) {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_FixedPoint; }
  static FixedPointTypeOverlay *get(bool s, int f, int b);
  static FixedPointTypeOverlay *get(llvm::Type *llvmtype, bool signd = true)
  {
    return llvm::dyn_cast<FixedPointTypeOverlay>(TypeOverlay::get(llvmtype, signd));
  }
  
  std::string toString() const override;
  llvm::Type *getBaseLLVMType(llvm::LLVMContext& ctxt) const override;
  
  inline bool getSigned() const { return isSigned; };
  inline int getPointPos() const { return fracBitsAmt; };
  inline int getSize() const { return bitsAmt; };
};

}


#endif


