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


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertInstruction(Module& m, Instruction *val)
{
  Value *res = Unsupported;
  
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    res = convertAlloca(alloca);
  } else if (LoadInst *load = dyn_cast<LoadInst>(val)) {
    res = convertLoad(load);
  } else if (StoreInst *store = dyn_cast<StoreInst>(val)) {
    res = convertStore(store);
  } else if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(val)) {
    res = convertGep(gep);
  } else if (PHINode *phi = dyn_cast<PHINode>(val)) {
    res = convertPhi(phi);
  } else if (SelectInst *select = dyn_cast<SelectInst>(val)) {
    res = convertSelect(select);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) { //llvm/include/llvm/IR/Instruction.def for more info
    if (instr->isBinaryOp()) {
      res = convertBinOp(instr);
    } else if (CastInst *cast = dyn_cast<CastInst>(instr)){
      res = convertCast(cast);
    } else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(val)) {
      res = convertCmp(fcmp);
    }
  }

  if (res==Unsupported) {
    res = fallback(dyn_cast<Instruction>(val));
  }

  return res ? res : ConversionError;
}


Value *FloatToFixed::convertAlloca(AllocaInst *alloca)
{
  Type *prevt = alloca->getAllocatedType();
  Type *newt = getFixedPointTypeForFloatType(prevt);

  Value *as = alloca->getArraySize();
  AllocaInst *newinst = new AllocaInst(newt, as, alloca->getAlignment(),
    alloca->hasName() ? alloca->getName() + ".fixp" : "fixp");
  
  newinst->setUsedWithInAlloca(alloca->isUsedWithInAlloca());
  newinst->setSwiftError(alloca->isSwiftError());
  newinst->insertAfter(alloca);
  return newinst;
}


Value *FloatToFixed::convertLoad(LoadInst *load)
{
  Value *ptr = load->getPointerOperand();
  Value *newptr = operandPool[ptr];
  if (newptr == ConversionError)
    return nullptr;
  assert(newptr && "a load can't be in the conversion queue just because");

  LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
    load->getAlignment(), load->getOrdering(), load->getSynchScope());
  newinst->insertAfter(load);
  return newinst;
}


Value *FloatToFixed::convertStore(StoreInst *store)
{
  Value *ptr = store->getPointerOperand();
  Value *newptr = operandPool[ptr];
  if (!newptr)
    newptr = ptr;
  else if (newptr == ConversionError)
    return nullptr;

  Value *val = store->getValueOperand();
  Value *newval = translateOrMatchOperand(val, store);
  if (!newval)
    return nullptr;

  if (newval->getType() != cast<PointerType>(newptr->getType())->getElementType()) {
    /* if the destination is to a floating point value and the source is not,
     * convert back to a float and store the converted value
     * we can't do that if the source is a pointer though; in that case bail out */
    if (newval->getType()->isPointerTy())
      return nullptr;
    newval = genConvertFixToFloat(newval,cast<PointerType>(newptr->getType())->getElementType());
  }
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSynchScope());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertGep(GetElementPtrInst *gep)
{
  IRBuilder <> builder (gep);
  Value *newval = translateOrMatchOperand(gep->getPointerOperand(), gep);
  if (!newval)
    return nullptr;

  std::vector<Value*> vals;
  for (auto a : gep->operand_values()) {
    vals.push_back(a);
  }
  vals.erase(vals.begin());

  ArrayRef <Value*> idxlist(vals);
  return builder.CreateInBoundsGEP(newval,idxlist);
}


Value *FloatToFixed::convertPhi(PHINode *load)
{
  if (!load->getType()->isFloatingPointTy()) {
    /* in the conversion chain the floating point number was converted to
     * an int at some point; we just upgrade the incoming values in place */

    /* if all of our incoming values were not converted, we want to propagate
     * that information across the phi. If at least one of them was converted
     * the phi is converted as well; otherwise it is not. */
    bool donesomething = false;

    for (int i=0; i<load->getNumIncomingValues(); i++) {
      Value *thisval = load->getIncomingValue(i);
      Value *newval = operandPool[thisval];
      if (newval && newval != ConversionError) {
        load->setIncomingValue(i, newval);
        donesomething = true;
      }
    }
    return donesomething ? load : nullptr;
  }

  /* if we have to do a type change, create a new phi node. The new type is for
   * sure that of a fixed point value; because the original type was a float
   * and thus all of its incoming values were floats */
  PHINode *newphi = PHINode::Create(getFixedPointType(load->getContext()),
    load->getNumIncomingValues());

  for (int i=0; i<load->getNumIncomingValues(); i++) {
    Value *thisval = load->getIncomingValue(i);
    BasicBlock *thisbb = load->getIncomingBlock(i);
    Value *newval = translateOrMatchOperand(thisval, load);
    if (!newval) {
      delete newphi;
      return nullptr;
    }
    newphi->addIncoming(newval, thisbb);
  }
  newphi->insertAfter(load);
  return newphi;
}


Value *FloatToFixed::convertSelect(SelectInst *sel)
{
  if (!sel->getType()->isFloatingPointTy()) {
    Value *newcond = operandPool[sel->getCondition()];
    Value *newtruev = operandPool[sel->getTrueValue()];
    Value *newfalsev = operandPool[sel->getFalseValue()];

    /* like phi, upgrade in place */
    if (newcond && newcond != ConversionError)
      sel->setCondition(newcond);
    if (newtruev && newtruev != ConversionError)
      sel->setTrueValue(newtruev);
    if (newfalsev && newfalsev != ConversionError)
      sel->setFalseValue(newfalsev);
    return sel;
  }

  /* otherwise create a new one */
  Value *newtruev = translateOrMatchOperand(sel->getTrueValue(), sel);
  Value *newfalsev = translateOrMatchOperand(sel->getFalseValue(), sel);
  Value *newcond = translateOrMatchOperand(sel->getCondition(), sel);
  if (!newtruev || !newfalsev || !newcond)
    return nullptr;

  SelectInst *newsel = SelectInst::Create(newcond, newtruev, newfalsev);
  newsel->insertAfter(sel);
  return newsel;
}


Value *FloatToFixed::convertBinOp(Instruction *instr)
{
  IRBuilder<> builder(instr->getNextNode());
  Instruction::BinaryOps ty;
  int opc = instr->getOpcode();
  /*le istruzioni Instruction::
    [Add,Sub,Mul,SDiv,UDiv,SRem,URem,Shl,LShr,AShr,And,Or,Xor]
    vengono gestite dalla fallback e non in questa funzione */

  Value *val1 = translateOrMatchOperand(instr->getOperand(0), instr);
  Value *val2 = translateOrMatchOperand(instr->getOperand(1), instr);
  if (!val1 || !val2)
    return nullptr;

  Type *fxt = getFixedPointType(instr->getContext());
  Type *dbfxt = Type::getIntNTy(instr->getContext(), bitsAmt*2);

  if (opc == Instruction::FAdd) {
    ty = Instruction::Add;
  } else if (opc == Instruction::FSub) {
    ty = Instruction::Sub;
  } else if (opc == Instruction::FMul) {

    Value *ext1 = builder.CreateSExt(val1,dbfxt);
    Value *ext2 = builder.CreateSExt(val2,dbfxt);

    Value *fixop = builder.CreateMul(ext1,ext2);
    return builder.CreateTrunc(
      builder.CreateAShr(fixop,ConstantInt::get(dbfxt,fracBitsAmt)),
      fxt);

  } else if (opc == Instruction::FDiv) {

    Value *ext1 = builder.CreateShl(
      builder.CreateSExt(val1,dbfxt),
      ConstantInt::get(dbfxt,fracBitsAmt)
      );
    Value *ext2 = builder.CreateSExt(val2,dbfxt);

    Value *fixop = builder.CreateSDiv(ext1,ext2);
    return builder.CreateTrunc(fixop,fxt);

  } else if (opc == Instruction::FRem) {
    ty = Instruction::SRem;
  } else {
    return Unsupported;
  }

  return builder.CreateBinOp(ty,val1,val2);
}


Value *FloatToFixed::convertCmp(FCmpInst *fcmp)
{
  IRBuilder<> builder (fcmp->getNextNode());
  CmpInst::Predicate ty;
  int pr = fcmp->getPredicate();
  bool swapped = false;

  //se unordered swappo, converto con la int, e poi mi ricordo di riswappare
  if (!CmpInst::isOrdered(fcmp->getPredicate())) {
    pr = fcmp->getInversePredicate();
    swapped = true;
  }

  if (pr == CmpInst::FCMP_OEQ){
    ty = CmpInst::ICMP_EQ;
  } else if (pr == CmpInst::FCMP_ONE) {
    ty = CmpInst::ICMP_NE;
  } else if (pr == CmpInst::FCMP_OGT) {
    ty = CmpInst::ICMP_SGT;
  } else if (pr == CmpInst::FCMP_OGE) {
    ty = CmpInst::ICMP_SGE;
  } else if (pr == CmpInst::FCMP_OLE) {
    ty = CmpInst::ICMP_SLE;
  } else if (pr == CmpInst::FCMP_OLT) {
    ty = CmpInst::ICMP_SLT;
  } else if (pr == CmpInst::FCMP_ORD) {
    ;
    //TODO gestione NaN
  } else if (pr == CmpInst::FCMP_TRUE) {
    /* there is no integer-only always-true / always-false comparison
     * operator... so we roll out our own by producing a tautology */
    return builder.CreateICmpEQ(
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0));
  } else if (pr == CmpInst::FCMP_FALSE) {
    return builder.CreateICmpNE(
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0),
      ConstantInt::get(Type::getInt32Ty(fcmp->getContext()),0));
  }


  if (swapped) {
    ty = CmpInst::getInversePredicate(ty);
  }

  Value *val1 = translateOrMatchOperand(fcmp->getOperand(0), fcmp);
  Value *val2 = translateOrMatchOperand(fcmp->getOperand(1), fcmp);

  return val1 && val2
    ? builder.CreateICmp(ty,val1,val2)
    : nullptr;
}


Value *FloatToFixed::convertCast(CastInst *cast)
{
  /* le istruzioni Instruction::
   * - [FPToSI,FPToUI,SIToFP,UIToFP] vengono gestite qui
   * - [Trunc,ZExt,SExt] vengono gestite dalla fallback e non qui
   * - [PtrToInt,IntToPtr,BitCast,AddrSpaceCast] potrebbero portare errori */

  IRBuilder<> builder(cast->getNextNode());
  Value *val = translateOrMatchOperand(cast->getOperand(0), cast);

  if (cast->getOpcode() == Instruction::FPToSI) {
    return builder.CreateSExtOrTrunc(
      builder.CreateAShr(val,ConstantInt::get(getFixedPointType(val->getContext()),fracBitsAmt)),
      cast->getType()
    );

  } else if (cast->getOpcode() == Instruction::FPToUI) {
    return builder.CreateZExtOrTrunc(
      builder.CreateAShr(val,ConstantInt::get(getFixedPointType(val->getContext()),fracBitsAmt)),
      cast->getType()
    );

  } else if (cast->getOpcode() == Instruction::SIToFP) {
    return builder.CreateShl(
      builder.CreateSExtOrTrunc(val,getFixedPointType(val->getContext())),
      ConstantInt::get(getFixedPointType(val->getContext()) ,fracBitsAmt)
    );

  } else if (cast->getOpcode() == Instruction::UIToFP) {
    return builder.CreateShl(
      builder.CreateZExtOrTrunc(val,getFixedPointType(val->getContext())),
      ConstantInt::get(getFixedPointType(val->getContext()) ,fracBitsAmt)
    );

  } else if (cast->getOpcode() == Instruction::FPTrunc ||
             cast->getOpcode() == Instruction::FPExt) {
    /* there is just one type of fixed point type; nothing to convert */
    return val;
  }

  return Unsupported;
}


Value *FloatToFixed::fallback(Instruction *unsupp)
{
  Value *fallval;
  Instruction *fixval;
  std::vector<Value *> newops;

  DEBUG(dbgs() << "[Fallback] attempt to wrap not supported operation:\n" << *unsupp << "\n");
  FallbackCount++;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);

    Value *cvtfallval = operandPool[fallval];
    if (cvtfallval == ConversionError || (cvtfallval && cvtfallval->getType()->isPointerTy())) {
      DEBUG(dbgs() << "  bail out on missing operand " << i+1 << " of " << n << "\n");
      return nullptr;
    }

    //se è stato precedentemente sostituito e non è un puntatore
    if (cvtfallval) {
      /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
        il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
        Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
      fixval = fallval->getType()->isFloatingPointTy()
        ? dyn_cast<Instruction>(genConvertFixToFloat(cvtfallval,fallval->getType()))
        : dyn_cast<Instruction>(cvtfallval);

      DEBUG(dbgs() << "  Substituted operand number : " << i+1 << " of " << n << "\n");
      newops.push_back(fixval);
    } else {
      newops.push_back(fallval);
    }
  }

  for (int i=0, n=unsupp->getNumOperands(); i<n; i++) {
    unsupp->setOperand(i, newops[i]);
  }
  DEBUG(dbgs() << "  mutated operands to:\n" << *unsupp << "\n");
  if (unsupp->getType()->isFloatingPointTy()) {
    Value *fallbackv = genConvertFloatToFix(unsupp, unsupp);
    if (unsupp->hasName())
      fallbackv->setName(unsupp->getName() + ".fallback");
    return fallbackv;
  }
  return unsupp;
}


