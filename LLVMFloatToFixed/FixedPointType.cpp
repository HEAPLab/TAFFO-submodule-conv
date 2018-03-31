#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
#include "FixedPointType.h"
#include "LLVMFloatToFixedPass.h"


using namespace llvm;
using namespace flttofix;


cl::opt<int> GlobalFracBitsAmt("fixpfracbitsamt", cl::value_desc("bits"),
  cl::desc("Default amount of fractional bits in fixed point numbers"),
  cl::init(16));
cl::opt<int> GlobalBitsAmt("fixpbitsamt", cl::value_desc("bits"),
  cl::desc("Default amount of bits in fixed point numbers"), cl::init(32));


FixedPointType::FixedPointType()
{
  this->isSigned = true;
  this->fracBitsAmt = GlobalFracBitsAmt;
  this->bitsAmt = GlobalBitsAmt;
}


FixedPointType::FixedPointType(Type *llvmtype, bool signd)
{
  this->isSigned = signd;
  if (isFloatType(llvmtype)) {
    this->fracBitsAmt = GlobalFracBitsAmt;
    this->bitsAmt = GlobalBitsAmt;
    return;
  } else if (llvmtype->isIntegerTy()) {
    this->fracBitsAmt = 0;
    this->bitsAmt = llvmtype->getIntegerBitWidth();
  } else {
    this->fracBitsAmt = 0;
    this->bitsAmt = 0;
  }
}


Type *FixedPointType::toLLVMType(LLVMContext& ctxt) const
{
  return Type::getIntNTy(ctxt, this->bitsAmt);
}


raw_ostream& operator<<(raw_ostream& stm, const FixedPointType& f)
{
  if (f.isSigned)
    stm << "s";
  else
    stm << "u";
  stm << f.bitsAmt - f.fracBitsAmt << "_" << f.fracBitsAmt << "fixp";
  return stm;
};


