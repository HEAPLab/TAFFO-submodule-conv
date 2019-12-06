#include "llvm/Support/Casting.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "InputInfo.h"


#ifndef __TYPE_OVERLAY_H__
#define __TYPE_OVERLAY_H__


namespace flttofix {


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
    TOK_FixedPoint
  };
  
private:
  const TypeOverlayKind Kind;
  
protected:
  explicit TypeOverlay(TypeOverlayKind K) : Kind(K) {}
  
public:
  TypeOverlay() = delete;
  TypeOverlayKind getKind() const { return Kind; }
  
  static TypeOverlay *get(llvm::Type *llvmtype, bool signd = true);
  static TypeOverlay *get(mdutils::MDInfo *mdnfo, int *enableConversion = nullptr);
  static TypeOverlay *get(mdutils::TType *mdtype);
  
  virtual std::string toString() const = 0;
  
  virtual llvm::Type *uniformToBaseLLVMType(llvm::LLVMContext& ctxt) const { assert(false && "not a scalar"); }
  
  virtual bool isUniform() const { return Kind != TOK_Struct; }
  virtual bool isVoid() const { return Kind == TOK_Void; }
  virtual bool isRecursivelyVoid() const { return isVoid(); }
  
  TypeOverlay *unwrapIndexList(llvm::Type *valType, const llvm::iterator_range<const llvm::Use*> indices);
  TypeOverlay *unwrapIndexList(llvm::Type *valType, llvm::ArrayRef<unsigned> indices);
};


class VoidTypeOverlay : public TypeOverlay {
protected:
  VoidTypeOverlay(): TypeOverlay(TOK_Void) {}
  
private:
  static VoidTypeOverlay Void;
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Void; }
  static VoidTypeOverlay *get() { return &(VoidTypeOverlay::Void); };
  
  std::string toString() const override { return "<void>"; }
  
  llvm::Type *uniformToBaseLLVMType(llvm::LLVMContext& ctxt) const override { assert(false && "VoidTypeOverlay cannot be converted to a LLVM type"); }
};


class StructTypeOverlay : public TypeOverlay {
  llvm::SmallVector<TypeOverlay *, 2> elements;
  
  static llvm::StringMap<StructTypeOverlay *> StructTypes;
  
protected:
  /** Struct type
   *  @param elems List of types, one for each struct field. All elements will be
   *     copied */
  StructTypeOverlay(const llvm::ArrayRef<TypeOverlay *>& elems);
  virtual ~StructTypeOverlay() {};
  
public:
  static bool classof(const TypeOverlay *O) { return O->getKind() == TOK_Struct; }
  static StructTypeOverlay *get(const llvm::ArrayRef<TypeOverlay *>& elems);

  std::string toString() const override;
  
  bool isRecursivelyVoid() const override;
  
  inline int size(void) const { return elements.size(); }
  inline TypeOverlay *item(int n) const { return elements[n]; }
};


}


llvm::raw_ostream& operator<<(llvm::raw_ostream& stm, const flttofix::TypeOverlay& f);


#endif
