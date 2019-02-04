#include <fstream>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"


#ifndef __FIXED_POINT_TYPE_H__
#define __FIXED_POINT_TYPE_H__


namespace flttofix {


class FixedPointType {
private:
  struct Primitive {
    bool isSigned;
    int fracBitsAmt;
    int bitsAmt;
    
    bool operator==(const Primitive& rhs) const {
      return this->isSigned == rhs.isSigned &&
        this->fracBitsAmt == rhs.fracBitsAmt &&
        this->bitsAmt == rhs.bitsAmt;
    };
    
    std::string toString() const;
  };
  std::vector<Primitive> elements;
  
public:
  FixedPointType();
  FixedPointType(bool s, int f, int b);
  FixedPointType(llvm::Type *llvmtype, bool signd = true);
  
  std::string toString() const;
  
  llvm::Type *scalarToLLVMType(llvm::LLVMContext& ctxt) const;
  inline bool& scalarIsSigned(void) {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].isSigned;
  };
  inline bool scalarIsSigned(void) const {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].isSigned;
  };
  inline int& scalarFracBitsAmt(void) {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].fracBitsAmt;
  };
  inline int scalarFracBitsAmt(void) const {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].fracBitsAmt;
  };
  inline int& scalarBitsAmt(void) {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].bitsAmt;
  };
  inline int scalarBitsAmt(void) const {
    assert(elements.size() == 1 && "fixed point type not a scalar");
    return elements[0].bitsAmt;
  };
  
  bool operator==(const FixedPointType& rhs) const;
};


}


llvm::raw_ostream& operator<<(llvm::raw_ostream& stm, const flttofix::FixedPointType& f);


#endif

