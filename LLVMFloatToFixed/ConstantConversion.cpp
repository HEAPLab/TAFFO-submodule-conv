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


Value *FloatToFixed::convertConstant(DenseMap<Value *, Value *>& op, Constant *flt)
{
  if (GlobalVariable *gvar = dyn_cast<GlobalVariable>(flt)) {
    return convertGlobalVariable(gvar);
  } else if (ConstantFP *fpc = dyn_cast<ConstantFP>(flt)) {
    return convertLiteral(fpc, nullptr);
  } else if (ConstantExpr *cexp = dyn_cast<ConstantExpr>(flt)) {
    return convertConstantExpr(op, cexp);
  }
  return nullptr;
}


Value *FloatToFixed::convertConstantExpr(DenseMap<Value *, Value *>& op, ConstantExpr *cexp)
{
  if (cexp->isGEPWithNoNotionalOverIndexing()) {
    Value *newval = op[cexp->getOperand(0)];
    if (!newval)
      return nullptr;
    Constant *newconst = dyn_cast<Constant>(newval);
    if (!newconst)
      return nullptr;
  
    std::vector<Constant *> vals;
    for (int i=1; i<cexp->getNumOperands(); i++) {
      vals.push_back(cexp->getOperand(i));
    }

    ArrayRef<Constant *> idxlist(vals);
    return ConstantExpr::getInBoundsGetElementPtr(nullptr, newconst, idxlist);
  }
  return nullptr;
}


Value *FloatToFixed::convertGlobalVariable(GlobalVariable *glob)
{
  Type *prevt = glob->getType()->getPointerElementType();
  Type *newt = getFixedPointTypeForFloatType(prevt);
  if (!newt)
    return nullptr;
  
  Constant *newinit = nullptr;
  if (newt->isAggregateType()) // TODO
    newinit = ConstantAggregateZero::get(newt);
  else
    newinit = ConstantInt::get(newt, 0);
  
  GlobalVariable *newglob = new GlobalVariable(*(glob->getParent()), newt, glob->isConstant(), glob->getLinkage(), newinit);
  return newglob;
}


Constant *FloatToFixed::convertLiteral(ConstantFP *fpc, Instruction *context)
{
  bool precise = false;
  APFloat val = fpc->getValueAPF();

  APFloat exp(pow(2.0, fracBitsAmt));
  exp.convert(val.getSemantics(), APFloat::rmTowardNegative, &precise);
  val.multiply(exp, APFloat::rmTowardNegative);

  integerPart fixval;
  APFloat::opStatus cvtres = val.convertToInteger(&fixval, bitsAmt, true,
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

  Type *intty = getFixedPointType(fpc->getType()->getContext());
  return ConstantInt::get(intty, fixval, true);
}


