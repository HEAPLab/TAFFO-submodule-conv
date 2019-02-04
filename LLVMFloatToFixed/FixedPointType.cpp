#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "FixedPointType.h"
#include "LLVMFloatToFixedPass.h"
#include <sstream>


using namespace llvm;
using namespace flttofix;


FixedPointType::FixedPointType()
{
  Primitive element = {false, 0, 0};
  elements.push_back(element);
}


FixedPointType::FixedPointType(bool s, int f, int b)
{
  Primitive element = {s, f, b};
  elements.push_back(element);
}


FixedPointType::FixedPointType(Type *llvmtype, bool signd)
{
  Primitive element;
  element.isSigned = signd;
  if (isFloatType(llvmtype)) {
    element.fracBitsAmt = 0;
    element.bitsAmt = 0;
  } else if (llvmtype->isIntegerTy()) {
    element.fracBitsAmt = 0;
    element.bitsAmt = llvmtype->getIntegerBitWidth();
  } else {
    element.isSigned = false;
    element.fracBitsAmt = 0;
    element.bitsAmt = 0;
  }
  elements.push_back(element);
}


Type *FixedPointType::scalarToLLVMType(LLVMContext& ctxt) const
{
  assert(elements.size() == 1 && "fixed point type not a scalar");
  return Type::getIntNTy(ctxt, elements[0].bitsAmt);
}


std::string FixedPointType::Primitive::toString() const
{
  std::stringstream stm;
  if (isSigned)
    stm << "s";
  else
    stm << "u";
  stm << bitsAmt - fracBitsAmt << "_" << fracBitsAmt << "fixp";
  return stm.str();
}


std::string FixedPointType::toString() const
{
  std::stringstream stm;
  
  if (elements.size() > 1)
    stm << '<';
  
  for (int i=0; i<elements.size(); i++) {
    stm << elements[i].toString();
    if (elements.size() > 1 && i != elements.size()-1)
      stm << ',';
  }
  
  if (elements.size() > 1)
    stm << '>';
  
  return stm.str();
}


raw_ostream& operator<<(raw_ostream& stm, const FixedPointType& f)
{
  stm << f.toString();
  return stm;
}


bool FixedPointType::operator==(const FixedPointType& rhs) const
{
  if (rhs.elements.size() != elements.size())
    return false;
  for (int i=0; i<elements.size(); i++) {
    if (!(rhs.elements[i] == elements[i]))
      return false;
  }
  return true;
}


