#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
#include "llvm/Support/CommandLine.h"


#ifndef __FIXED_POINT_TYPE_H__
#define __FIXED_POINT_TYPE_H__


namespace flttofix {


struct FixedPointType {
  bool isSigned;
  int fracBitsAmt;
  int bitsAmt;
  
  FixedPointType();
  llvm::Type *toLLVMType(llvm::LLVMContext& ctxt);
  
  bool operator==(const FixedPointType& rhs) {
    return this->isSigned == rhs.isSigned &&
      this->fracBitsAmt == rhs.fracBitsAmt &&
      this->bitsAmt == rhs.bitsAmt;
  };
};


}


#endif

