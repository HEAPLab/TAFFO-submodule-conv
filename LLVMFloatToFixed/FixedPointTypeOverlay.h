#ifndef __FIXED_POINT_TYPE_OVERLAY_H__
#define __FIXED_POINT_TYPE_OVERLAY_H__

#include "TypeOverlay.h"


namespace flttofix {

class FloatTypeOverlay;

class FixedPointTypeOverlay : public UniformTypeOverlay {
  bool isSigned;
  int fracBitsAmt;
  int bitsAmt;
  
protected:
  FixedPointTypeOverlay(FloatToFixed *C, bool s, int f, int b): UniformTypeOverlay(TOK_FixedPoint, C), isSigned(s), fracBitsAmt(f), bitsAmt(b) {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_FixedPoint; }
  static FixedPointTypeOverlay *get(FloatToFixed *C, bool s, int f, int b);
  static FixedPointTypeOverlay *get(FloatToFixed *C, llvm::Type *llvmtype, bool signd = true)
  {
    return llvm::dyn_cast<FixedPointTypeOverlay>(TypeOverlay::get(C, llvmtype, signd));
  }
  
  std::string toString() const override;
  llvm::Type *getBaseLLVMType(llvm::LLVMContext& ctxt) const override;
  
  inline bool getSigned() const { return isSigned; };
  inline int getPointPos() const { return fracBitsAmt; };
  inline int getSize() const { return bitsAmt; };
  
  using UniformTypeOverlay::genCastFrom;
  llvm::Value *genCastFrom(llvm::Value *I, const UniformTypeOverlay *IType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genCastFrom(llvm::Value *I, const FixedPointTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
  llvm::Value *genCastFrom(llvm::Value *I, const FloatTypeOverlay *IType, llvm::Instruction *IInsertBefore) const;
  
  llvm::Value *genCastOnlyIfNotFixedPoint(llvm::Value *I, const UniformTypeOverlay *IType, const FixedPointTypeOverlay **OutIType, llvm::Instruction *IInsertBefore) const;
  
  llvm::Value *genAdd(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genSub(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genRem(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genMul(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genDiv(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const override;
  llvm::Value *genCmp(llvm::CmpInst::Predicate Pred, llvm::Value *IThis, llvm::Value *IOther, const UniformTypeOverlay *OtherType, llvm::Instruction *IInsertBefore) const override;
};

}


#endif


