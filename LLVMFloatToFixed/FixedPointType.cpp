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


Type *FixedPointType::toLLVMType(LLVMContext& ctxt)
{
  return Type::getIntNTy(ctxt, this->bitsAmt);
}


