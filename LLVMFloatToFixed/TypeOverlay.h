#ifndef __TYPE_OVERLAY_H__
#define __TYPE_OVERLAY_H__

#include "llvm/Support/Casting.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "InputInfo.h"


namespace flttofix {


struct FloatToFixed;

/**
 * TypeOverlay is a class which augments a llvm::Type with additional information
 * related to a type change operation that Conversion must perform.
 *
 * Each subclass of TypeOverlay is used to describe a conversion to a specific family
 * of types.
 *
 * All TypeOverlays are singletons with opaque ownership, and thus they must not be
 * memory-managed or explicitly constructed. Treat them as pointers, and use the most
 * appropriate `get` static factory method instead.
 * This implies that equality of a TypeOverlay can be checked through pointer equality.
 */
class TypeOverlay {
public:
  enum TypeOverlayKind {
    TOK_Void,
    TOK_Struct,
    TOK_Uniform_Begin,
    TOK_FixedPoint,
    TOK_Float,
    TOK_Uniform_End
  };
  
private:
  const TypeOverlayKind Kind;
  
protected:
  explicit TypeOverlay(TypeOverlayKind K, FloatToFixed *C) : Kind(K), Context(C) {}
  FloatToFixed * const Context;
  
public:
  TypeOverlay() = delete;
  TypeOverlayKind getKind() const { return Kind; }
  FloatToFixed *getContext() const { return Context; }
  
  static TypeOverlay *get(FloatToFixed *C, llvm::Type *llvmtype, bool signd = true);
  static TypeOverlay *get(FloatToFixed *C, mdutils::MDInfo *mdnfo, int *enableConversion = nullptr);
  static TypeOverlay *get(FloatToFixed *C, mdutils::TType *mdtype);
  
  virtual std::string toString() const = 0;
  
  virtual bool isUniform() const { return Kind != TOK_Struct; }
  virtual bool isVoid() const { return Kind == TOK_Void; }
  virtual bool isRecursivelyVoid() const { return isVoid(); }
  
  TypeOverlay *unwrapIndexList(llvm::Type *valType, const llvm::iterator_range<const llvm::Use*> indices);
  TypeOverlay *unwrapIndexList(llvm::Type *valType, llvm::ArrayRef<unsigned> indices);
  
  llvm::Type *overlayOnBaseType(llvm::Type *T, bool *performedReplacement = nullptr) const;
};


class VoidTypeOverlay : public TypeOverlay {
protected:
  VoidTypeOverlay(FloatToFixed *C): TypeOverlay(TOK_Void, C) {}
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Void; }
  static VoidTypeOverlay *get(FloatToFixed *C);
  
  std::string toString() const override { return "<void>"; }
};


class StructTypeOverlay : public TypeOverlay {
  llvm::SmallVector<TypeOverlay *, 2> elements;
  
protected:
  /** Struct type
   *  @param elems List of types, one for each struct field. All elements will be
   *     copied */
  StructTypeOverlay(FloatToFixed *C, const llvm::ArrayRef<TypeOverlay *>& elems);
  virtual ~StructTypeOverlay() {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Struct; }
  static StructTypeOverlay *get(FloatToFixed *C, const llvm::ArrayRef<TypeOverlay *>& elems);

  std::string toString() const override;
  
  bool isRecursivelyVoid() const override;
  
  inline int size(void) const { return elements.size(); }
  inline TypeOverlay *item(int n) const { return elements[n]; }
};


class UniformTypeOverlay: public TypeOverlay {
protected:
  UniformTypeOverlay(TypeOverlayKind kind, FloatToFixed *C): TypeOverlay(kind, C) { assert(TOK_Uniform_Begin <= kind && kind << TOK_Uniform_End); };
  
public:
  static bool classof(const TypeOverlay *O) { return TOK_Uniform_Begin <= O->getKind() && O->getKind() <= TOK_Uniform_End; }
  
  virtual llvm::Type *getBaseLLVMType(llvm::LLVMContext& ctxt) const = 0;
  
  virtual llvm::Value *genCastFrom(llvm::Value *I, const UniformTypeOverlay *IType) const;
  virtual llvm::Value *genCastFrom(llvm::Instruction *I, const UniformTypeOverlay *IType) const;
  virtual llvm::Value *genCastFrom(llvm::Argument *A, const UniformTypeOverlay *IType) const;
  virtual llvm::Value *genCastFrom(llvm::Value *I, const UniformTypeOverlay *IType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genAdd(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genSub(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genRem(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genMul(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genDiv(llvm::Value *A, const UniformTypeOverlay *AType, llvm::Value *B, const UniformTypeOverlay *BType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
  virtual llvm::Value *genCmp(llvm::CmpInst::Predicate Pred, llvm::Value *IThis, llvm::Value *IOther, const UniformTypeOverlay *OtherType, llvm::Instruction *IInsertBefore) const { assert(false && "!IMPL"); }
};


}


llvm::raw_ostream& operator<<(llvm::raw_ostream& stm, const flttofix::TypeOverlay& f);


#endif
