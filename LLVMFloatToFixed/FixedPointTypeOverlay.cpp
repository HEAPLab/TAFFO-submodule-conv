#include <sstream>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "TypeUtils.h"
#include "FixedPointTypeOverlay.h"

using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


llvm::DenseMap<uint64_t, FixedPointTypeOverlay *> FixedPointTypeOverlay::FXTypes;


FixedPointTypeOverlay *FixedPointTypeOverlay::get(bool s, int f, int b)
{
  assert(std::abs(f) < (1 << 28) && std::abs(b) < (1 << 28) && "exceeded max size = 268435455 bits");
  assert(b > 0 && "total number of bits in representation must be > 0");
  uint64_t key =
    (((uint64_t)f & 0xFFFFFFF) << (28+1)) +
    (((uint64_t)b & 0xFFFFFFF) << (1)) +
    (s & 0x1);
  auto match = FixedPointTypeOverlay::FXTypes.find(key);
  if (match != FixedPointTypeOverlay::FXTypes.end())
    return match->getSecond();
  
  FixedPointTypeOverlay *res = new FixedPointTypeOverlay(s, f, b);
  FixedPointTypeOverlay::FXTypes[key] = res;
  return res;
}


std::string FixedPointTypeOverlay::toString() const
{
  std::stringstream stm;
  
  if (isSigned)
    stm << "s";
  else
    stm << "u";
  stm << bitsAmt - fracBitsAmt << "_" << fracBitsAmt << "fixp";
  
  return stm.str();
}


Type *FixedPointTypeOverlay::uniformToBaseLLVMType(LLVMContext& ctxt) const
{
  return Type::getIntNTy(ctxt, bitsAmt);
}


