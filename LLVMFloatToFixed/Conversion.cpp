#include <cmath>
#include <cassert>
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
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"

using namespace llvm;
using namespace flttofix;
using namespace taffo;


Value *ConversionError = (Value *)(&ConversionError);
Value *Unsupported = (Value *)(&Unsupported);


void FloatToFixed::performConversion(
  Module& m,
  std::vector<Value*>& q)
{
  
  for (auto i = q.begin(); i != q.end();) {
    Value *v = *i;
    
    if (CallInst *anno = dyn_cast<CallInst>(v)) {
      if (anno->getCalledFunction()) {
        if (anno->getCalledFunction()->getName() == "llvm.var.annotation") {
          anno->eraseFromParent();
          i = q.erase(i);
          continue;
        }
      }
    }
    
    LLVM_DEBUG(dbgs() << "* performConversion *\n");
    LLVM_DEBUG(dbgs() << "  [no conv ] " << valueInfo(v)->noTypeConversion << "\n");
    LLVM_DEBUG(dbgs() << "  [value   ] " << *v << "\n");
    if (Instruction *i = dyn_cast<Instruction>(v))
      LLVM_DEBUG(dbgs() << "  [function] " << i->getFunction()->getName() << "\n");
    
    Value *newv = convertSingleValue(m, v, valueInfo(v)->fixpType);
    if (newv) {
      operandPool[v] = newv;
    }
    
    if (newv && newv != ConversionError) {
      LLVM_DEBUG(dbgs() << "  [output  ] " << *newv << "\n");
      if (newv != v && isa<Instruction>(newv) && isa<Instruction>(v)) {
        Instruction *newinst = dyn_cast<Instruction>(newv);
        Instruction *oldinst = dyn_cast<Instruction>(v);
        newinst->setDebugLoc(oldinst->getDebugLoc());
      }
      cpMetaData(newv,v);
      if (newv != v) {
        if (hasInfo(newv)) {
          LLVM_DEBUG(dbgs() << "warning: output has valueInfo already from a previous conversion\n");
        } else {
          *newValueInfo(newv) = *valueInfo(v);
        }
      }
    } else {
      LLVM_DEBUG(dbgs() << "  [output  ] CONVERSION ERROR\n");
    }
    i++;
  }
}


Value *FloatToFixed::createPlaceholder(Type *type, BasicBlock *where, StringRef name)
{
  IRBuilder<> builder(where, where->getFirstInsertionPt());
  AllocaInst *alloca = builder.CreateAlloca(type);
  return builder.CreateLoad(type, alloca, name);
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, Value *val, TypeOverlay *&fixpt)
{
  Value *res = Unsupported;
  
  if (valueInfo(val)->isArgumentPlaceholder) {
    return matchOp(val);
  } else if (Constant *con = dyn_cast<Constant>(val)) {
    /* Since constants never change, there is never anything to substitute
     * in them */
    if (!valueInfo(con)->noTypeConversion)
      res = convertConstant(con, fixpt, TypeMatchPolicy::RangeOverHintMaxFrac);
    else
      res = con;
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) {
    res = convertInstruction(m, instr, fixpt);
  } else if (Argument *argument = dyn_cast<Argument>(val)) {
    if (isFloatType(argument->getType()))
      res = translateOrMatchOperand(val, fixpt, nullptr);
    else
      res = val;
  }
  
  return res ? res : ConversionError;
}


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(Value *val, TypeOverlay *&iofixpt, Instruction *ip, TypeMatchPolicy typepol)
{
  if (typepol == TypeMatchPolicy::ForceHint) {
    TypeOverlay *origfixpt = iofixpt;
    llvm::Value *tmp = translateOrMatchOperand(val, iofixpt, ip, TypeMatchPolicy::RangeOverHintMaxFrac);
    return genConvertFixedToFixed(tmp, cast<FixedPointTypeOverlay>(iofixpt), cast<FixedPointTypeOverlay>(origfixpt), ip);
  }
  
  assert(val->getType()->getNumContainedTypes() == 0 && "translateOrMatchOperand val is not a scalar value");
  Value *res = operandPool[val];
  if (res) {
    if (res == ConversionError)
      /* the value should have been converted but it hasn't; bail out */
      return nullptr;
    
    if (!valueInfo(val)->noTypeConversion) {
      /* the value has been successfully converted to fixed point in a previous step */
      iofixpt = fixPType(res);
      return res;
    }
    
    /* The value has changed but may not a fixed point */
    if (!res->getType()->isFloatingPointTy())
      /* Don't attempt to convert ints/pointers to fixed point */
      return res;
    /* Otherwise convert to fixed point the value */
    val = res;
  }

  assert(val->getType()->isFloatingPointTy());
  
  /* try the easy cases first
   *   this is essentially duplicated from genConvertFloatToFix because once we
   * enter that function iofixpt cannot change anymore
   *   in other words, by duplicating this logic here we potentially avoid a loss
   * of range if the suggested iofixpt is not enough for the value */
  if (Constant *c = dyn_cast<Constant>(val)) {
    Value *res = convertConstant(c, iofixpt, typepol);
    return res;
  } else if (SIToFPInst *instr = dyn_cast<SIToFPInst>(val)) {
    Value *intparam = instr->getOperand(0);
    iofixpt = TypeOverlay::get(this, intparam->getType(), true);
    return intparam;
  } else if (UIToFPInst *instr = dyn_cast<UIToFPInst>(val)) {
    Value *intparam = instr->getOperand(0);
    iofixpt = TypeOverlay::get(this, intparam->getType(), true);
    return intparam;
  }
  
  /* not an easy case; check if the value has a range metadata
   * from VRA before giving up and using the suggested type */
  mdutils::MDInfo *mdi = mdutils::MetadataManager::getMetadataManager().retrieveMDInfo(val);
  if (mdutils::InputInfo *ii = dyn_cast_or_null<mdutils::InputInfo>(mdi)) {
    if (ii->IRange) {
      FixedPointTypeGenError err;
      FixedPointTypeOverlay *FXT = cast<FixedPointTypeOverlay>(iofixpt);
      mdutils::FPType fpt = taffo::fixedPointTypeFromRange(*(ii->IRange), &err, FXT->getSize());
      if (err != FixedPointTypeGenError::InvalidRange)
        iofixpt = TypeOverlay::get(this, &fpt);
    }
  }
  
  return genConvertFloatToFix(val, cast<FixedPointTypeOverlay>(iofixpt), ip);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt, FixedPointTypeOverlay *fixpt, Instruction *ip)
{
  assert(flt->getType()->isFloatingPointTy() && "genConvertFloatToFixed called on a non-float scalar");
  
  if (Constant *c = dyn_cast<Constant>(flt)) {
    TypeOverlay *fixptcopy = fixpt;
    Value *res = convertConstant(c, fixptcopy, TypeMatchPolicy::ForceHint);
    assert(fixptcopy == fixpt && "why is there a pointer here?");
    return res;
  }

  FloatTypeOverlay *fltt = FloatTypeOverlay::get(this, flt->getType());
  if (ip)
    return fixpt->genCastFrom(flt, fltt, ip);
  return fixpt->genCastFrom(flt, fltt);
}


Value *FloatToFixed::genConvertFixedToFixed(Value *fix, FixedPointTypeOverlay *srct, FixedPointTypeOverlay *destt, Instruction *ip)
{
  if (ip)
    return destt->genCastFrom(fix, srct, ip);
  return destt->genCastFrom(fix, srct);
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, FixedPointTypeOverlay *fixpt, Type *destt)
{
  FloatTypeOverlay *destto = FloatTypeOverlay::get(this, destt);
  return destto->genCastFrom(fix, fixpt);
}


Type *FloatToFixed::getLLVMFixedPointTypeForFloatType(Type *srct, TypeOverlay *baset, bool *hasfloats)
{
  return baset->overlayOnBaseType(srct, hasfloats);
}


Type *FloatToFixed::getLLVMFixedPointTypeForFloatValue(Value *val)
{
  TypeOverlay *fpt = fixPType(val);
  return getLLVMFixedPointTypeForFloatType(val->getType(), fpt);
}



