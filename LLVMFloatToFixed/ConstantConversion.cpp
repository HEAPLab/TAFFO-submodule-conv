#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>
#include <cassert>
#include "LLVMFloatToFixedPass.h"

using namespace llvm;
using namespace flttofix;

#define defaultFixpType @SYNTAX_ERROR@


Constant *FloatToFixed::convertConstant(Constant *flt, FixedPointType& fixpt)
{
  if (GlobalVariable *gvar = dyn_cast<GlobalVariable>(flt)) {
    return convertGlobalVariable(gvar, fixpt);
  } else if (ConstantFP *fpc = dyn_cast<ConstantFP>(flt)) {
    return convertLiteral(fpc, nullptr, fixpt);
  } else if (auto cag = dyn_cast<ConstantAggregateZero>(flt)) {
    Type *newt = getLLVMFixedPointTypeForFloatType(flt->getType(), fixpt);
    return ConstantAggregateZero::get(newt);
  } else if (ConstantExpr *cexp = dyn_cast<ConstantExpr>(flt)) {
    return convertConstantExpr(cexp, fixpt);
  }
  return nullptr;
}


Constant *FloatToFixed::convertConstantExpr(ConstantExpr *cexp, FixedPointType& fixpt)
{
  if (cexp->isGEPWithNoNotionalOverIndexing()) {
    Value *newval = operandPool[cexp->getOperand(0)];
    if (!newval)
      return nullptr;
    Constant *newconst = dyn_cast<Constant>(newval);
    if (!newconst)
      return nullptr;
    
    fixpt = fixPType(newval);
  
    std::vector<Constant *> vals;
    for (int i=1; i<cexp->getNumOperands(); i++) {
      vals.push_back(cexp->getOperand(i));
    }

    ArrayRef<Constant *> idxlist(vals);
    return ConstantExpr::getInBoundsGetElementPtr(nullptr, newconst, idxlist);
  }
  return nullptr;
}


Constant *FloatToFixed::convertGlobalVariable(GlobalVariable *glob, FixedPointType& fixpt)
{
  bool hasfloats;
  Type *prevt = glob->getType()->getPointerElementType();
  Type *newt = getLLVMFixedPointTypeForFloatType(prevt, fixpt, &hasfloats);
  if (!newt)
    return nullptr;
  if (!hasfloats)
    return glob;
  
  Constant *oldinit = glob->getInitializer();
  Constant *newinit = nullptr;
  if (oldinit)
    newinit = convertConstant(oldinit, fixpt);
  
  GlobalVariable *newglob = new GlobalVariable(*(glob->getParent()), newt, glob->isConstant(), glob->getLinkage(), newinit);
  newglob->setAlignment(glob->getAlignment());
  newglob->setName(glob->getName() + ".fixp");
  return newglob;
}


Constant *FloatToFixed::convertLiteral(ConstantFP *fpc, Instruction *context, const FixedPointType& fixpt)
{
  bool precise = false;
  APFloat val = fpc->getValueAPF();

  APFloat exp(pow(2.0, fixpt.fracBitsAmt));
  exp.convert(val.getSemantics(), APFloat::rmTowardNegative, &precise);
  val.multiply(exp, APFloat::rmTowardNegative);

  integerPart fixval;
  APFloat::opStatus cvtres = val.convertToInteger(&fixval, fixpt.bitsAmt, true,
    APFloat::rmTowardNegative, &precise);
  
  if (cvtres != APFloat::opStatus::opOK && context) {
    SmallVector<char, 64> valstr;
    val.toString(valstr);
    std::string valstr2(valstr.begin(), valstr.end());
    OptimizationRemarkEmitter ORE(context->getFunction());
    if (cvtres == APFloat::opStatus::opInexact) {
      ORE.emit(OptimizationRemark(DEBUG_TYPE, "ImpreciseConstConversion", context) <<
        "fixed point conversion of constant " << valstr2 << " is not precise\n");
    } else {
      ORE.emit(OptimizationRemark(DEBUG_TYPE, "ConstConversionFailed", context) <<
        "impossible to convert constant " << valstr2 << " to fixed point\n");
      return nullptr;
    }
  }

  Type *intty = fixpt.toLLVMType(fpc->getContext());
  return ConstantInt::get(intty, fixval, true);
}


