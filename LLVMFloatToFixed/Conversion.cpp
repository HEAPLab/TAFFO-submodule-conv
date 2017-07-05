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


Value *ConversionError = (Value *)(&ConversionError);
Value *Unsupported = (Value *)(&Unsupported);


void FloatToFixed::performConversion(Module& m, const std::vector<Value*>& q)
{
  DenseMap<Value *, Value *> convertedPool;

  for (Value *v: q) {
    Value *newv = convertSingleValue(m, convertedPool, v);
    if (newv && newv != ConversionError) {
      convertedPool.insert({v, newv});
    } else {
      DEBUG(dbgs() << "warning: ";
            v->print(dbgs());
            dbgs() << " not converted\n";);

      convertedPool.insert({v, ConversionError});
    }
  }

  /* remove all successfully converted stores. MAY PRODUCE INCORRECT RESULTS */
  //TODO: better logic here!!
  for (Value *v: q) {
    auto *i = dyn_cast<StoreInst>(v);
    auto *convi = convertedPool[v];
    if (convi && convi != ConversionError && i)
      i->eraseFromParent();
  }
}


/* also inserts the new value in the basic blocks, alongside the old one */
Value *FloatToFixed::convertSingleValue(Module& m, DenseMap<Value *, Value *>& operandPool, Value *val)
{
  Value *res = Unsupported;
  if (AllocaInst *alloca = dyn_cast<AllocaInst>(val)) {
    res = convertAlloca(alloca);
  } else if (LoadInst *load = dyn_cast<LoadInst>(val)) {
    res = convertLoad(operandPool, load);
  } else if (StoreInst *store = dyn_cast<StoreInst>(val)) {
    res = convertStore(operandPool, store);
  } else if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(val)) {
    res = convertGep(operandPool, gep);
  } else if (PHINode *phi = dyn_cast<PHINode>(val)) {
    res = convertPhi(operandPool, phi);
  } else if (SelectInst *select = dyn_cast<SelectInst>(val)) {
    res = convertSelect(operandPool, select);
  } else if (Instruction *instr = dyn_cast<Instruction>(val)) { //llvm/include/llvm/IR/Instruction.def for more info
    if (instr->isBinaryOp()) {
      res = convertBinOp(operandPool,instr);
    } else if (CastInst *cast = dyn_cast<CastInst>(instr)){
      res = convertCast(operandPool,cast);
    } else if (FCmpInst *fcmp = dyn_cast<FCmpInst>(val)) {
      res = convertCmp(operandPool,fcmp);
    }
  }

  if (res==Unsupported) {
    res = fallback(operandPool,dyn_cast<Instruction>(val));
  }

  return res;
}


Value *FloatToFixed::convertAlloca(AllocaInst *alloca)
{
  Type *prevt = alloca->getAllocatedType();
  Type *newt;
  std::vector<int> dim;

  while (prevt->isArrayTy()) {
    dim.push_back(prevt->getArrayNumElements());
    prevt = prevt->getArrayElementType();
  }

  if (!prevt->isFloatingPointTy()) {
    DEBUG(dbgs() << *alloca << " does not allocate a float or an array of floats\n");
    return nullptr;
  }

  newt = getFixedPointType(prevt->getContext());
  for (int i=dim.size()-1;i>=0;i--) {
    newt = ArrayType::get(newt, dim[i]);
  }

  Value *as = alloca->getArraySize();
  AllocaInst *newinst = new AllocaInst(newt, as, alloca->getAlignment(),
    "__" + alloca->getName() + "_fixp");
  newinst->setUsedWithInAlloca(alloca->isUsedWithInAlloca());
  newinst->setSwiftError(alloca->isSwiftError());
  newinst->insertAfter(alloca);
  return newinst;
}


Value *FloatToFixed::convertLoad(DenseMap<Value *, Value *>& op, LoadInst *load)
{
  Value *ptr = load->getPointerOperand();
  Value *newptr = op[ptr];
  if (newptr == ConversionError)
    return nullptr;
  assert(newptr && "a load can't be in the conversion queue just because");

  LoadInst *newinst = new LoadInst(newptr, Twine(), load->isVolatile(),
    load->getAlignment(), load->getOrdering(), load->getSynchScope());
  newinst->insertAfter(load);
  return newinst;
}


Value *FloatToFixed::convertStore(DenseMap<Value *, Value *>& op, StoreInst *store)
{
  Value *ptr = store->getPointerOperand();
  Value *newptr = op[ptr];
  if (!newptr)
    newptr = ptr;
  else if (newptr == ConversionError)
    return nullptr;

  Value *val = store->getValueOperand();
  Value *newval = translateOrMatchOperand(op, val, store);
  if (!newval)
    return nullptr;

  if (newval->getType() != cast<PointerType>(newptr->getType())->getElementType()) {
    //se l'area di memoria non è float , riporto il valore in float per memorizzarlo correttamente
    newval = genConvertFixToFloat(newval,cast<PointerType>(newptr->getType())->getElementType());
  }
  StoreInst *newinst = new StoreInst(newval, newptr, store->isVolatile(),
    store->getAlignment(), store->getOrdering(), store->getSynchScope());
  newinst->insertAfter(store);
  return newinst;
}


Value *FloatToFixed::convertGep(DenseMap<Value *, Value *>& op, GetElementPtrInst *gep)
{
  IRBuilder <> builder (gep);
  Value *newval = translateOrMatchOperand(op, gep->getPointerOperand(), gep);

  std::vector<Value*> vals;
  for (auto a : gep->operand_values() ) {
    vals.push_back(a);
  }
  vals.erase(vals.begin());

  ArrayRef <Value*> idxlist(vals);
  return builder.CreateInBoundsGEP(newval,idxlist);
}


Value *FloatToFixed::convertPhi(DenseMap<Value *, Value *>& op, PHINode *load)
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
      Value *newval = op[thisval];
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
    Value *newval = translateOrMatchOperand(op, thisval, load);
    if (!newval) {
      delete newphi;
      return nullptr;
    }
    newphi->addIncoming(newval, thisbb);
  }
  newphi->insertAfter(load);
  return newphi;
}


Value *FloatToFixed::convertSelect(DenseMap<Value *, Value *>& op, SelectInst *sel)
{
  if (!sel->getType()->isFloatingPointTy()) {
    Value *newcond = op[sel->getCondition()];
    Value *newtruev = op[sel->getTrueValue()];
    Value *newfalsev = op[sel->getFalseValue()];

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
  Value *newtruev = translateOrMatchOperand(op, sel->getTrueValue(), sel);
  Value *newfalsev = translateOrMatchOperand(op, sel->getFalseValue(), sel);
  Value *newcond = translateOrMatchOperand(op, sel->getCondition(), sel);
  if (!newtruev || !newfalsev || !newcond)
    return nullptr;

  SelectInst *newsel = SelectInst::Create(newcond, newtruev, newfalsev);
  newsel->insertAfter(sel);
  return newsel;
}


Value *FloatToFixed::convertBinOp(DenseMap<Value *, Value *>& op, Instruction *instr)
{
  IRBuilder<> builder(instr->getNextNode());
  Instruction::BinaryOps ty;
  int opc = instr->getOpcode();
  /*le istruzioni Instruction::
    [Add,Sub,Mul,SDiv,UDiv,SRem,URem,Shl,LShr,AShr,And,Or,Xor]
    vengono gestite dalla fallback e non in questa funzione */

  Value *val1 = translateOrMatchOperand(op, instr->getOperand(0), instr);
  Value *val2 = translateOrMatchOperand(op, instr->getOperand(1), instr);
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
      ConstantInt::get(dbfxt,bitsAmt)
      );
    Value *ext2 = builder.CreateShl(
      builder.CreateSExt(val2,dbfxt),
      ConstantInt::get(dbfxt,fracBitsAmt)
      );

    Value *fixop = builder.CreateSDiv(ext1,ext2);
    return builder.CreateTrunc(fixop,fxt);

  } else if (opc == Instruction::FRem) {
    ty = Instruction::SRem;
  } else {
    return Unsupported;
  }

  return builder.CreateBinOp(ty,val1,val2);
}


Value *FloatToFixed::convertCmp(DenseMap<Value *, Value *>& op, FCmpInst *fcmp)
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

  Value *val1 = translateOrMatchOperand(op, fcmp->getOperand(0), fcmp);
  Value *val2 = translateOrMatchOperand(op, fcmp->getOperand(1), fcmp);

  return val1 && val2
    ? builder.CreateICmp(ty,val1,val2)
    : nullptr;
}


Value *FloatToFixed::convertCast(DenseMap<Value *, Value *>& op, CastInst *cast)
{
  /* le istruzioni Instruction::
   * - [FPToSI,FPToUI,SIToFP,UIToFP] vengono gestite qui
   * - [Trunc,ZExt,SExt] vengono gestite dalla fallback e non qui
   * - [PtrToInt,IntToPtr,BitCast,AddrSpaceCast] potrebbero portare errori */
  
  IRBuilder<> builder(cast->getNextNode());
  Value *val = translateOrMatchOperand(op, cast->getOperand(0), cast);
  
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


Value *FloatToFixed::fallback(DenseMap<Value *, Value *>& op, Instruction *unsupp)
{
  Value *fallval;
  Instruction *fixval;
  std::vector<Value *> newops;

  DEBUG(dbgs() << "[Fallback] attempt to wrap not supported operation:\n" << *unsupp << "\n");
  FallbackCount++;

  for (int i=0,n=unsupp->getNumOperands();i<n;i++) {
    fallval = unsupp->getOperand(i);

    Value *cvtfallval = op[fallval];
    if (cvtfallval == ConversionError || fallval->getType()->isPointerTy()) {
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


/* do not use on pointer operands */
Value *FloatToFixed::translateOrMatchOperand(DenseMap<Value *, Value *>& op, Value *val, Instruction *ip)
{
  Value *res = op[val];
  if (res) {
    if (res != ConversionError)
      /* the value has been successfully converted in a previous step */
      return res;
    else
      /* the value should have been converted but it hasn't; bail out */
      return nullptr;
  }

  if (!val->getType()->isFloatingPointTy())
    /* doesn't need to be converted; return as is */
    return val;

  return genConvertFloatToFix(val, ip);
}


Value *FloatToFixed::genConvertFloatToFix(Value *flt, Instruction *ip)
{
  if (ConstantFP *fpc = dyn_cast<ConstantFP>(flt)) {
    return convertFloatConstantToFixConstant(fpc);
  }
  
  FloatToFixCount++;
  
  if (Instruction *i = dyn_cast<Instruction>(flt))
    ip = i->getNextNode();
  assert(ip && "ip is mandatory if not passing an instruction/constant value");

  /* insert new instructions before ip */
  IRBuilder<> builder(ip);
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFPToSI(
    builder.CreateFMul(
      ConstantFP::get(flt->getType(), twoebits),
      flt),
    getFixedPointType(flt->getContext()));
}


Constant *FloatToFixed::convertFloatConstantToFixConstant(ConstantFP *fpc)
{
  bool precise = false;
  APFloat val = fpc->getValueAPF();

  APFloat exp(pow(2.0, fracBitsAmt));
  exp.convert(val.getSemantics(), APFloat::rmTowardNegative, &precise);
  val.multiply(exp, APFloat::rmTowardNegative);

  integerPart fixval;
  APFloat::opStatus cvtres = val.convertToInteger(&fixval, bitsAmt, true,
    APFloat::rmTowardNegative, &precise);
  if (cvtres != APFloat::opStatus::opOK) {
    SmallVector<char, 64> valstr;
    val.toString(valstr);
    Twine valstr2 = valstr;
    // TODO: emit a proper warning! we don't do it now because it's such a hassle
    if (cvtres == APFloat::opStatus::opInexact) {
      errs() << "fixed point conversion of constant " <<
        valstr2 << " is not precise\n";
    } else {
      errs() << "impossible to convert constant " <<
        valstr2 << " to fixed point\n";
      return nullptr;
    }
  }

  Type *intty = getFixedPointType(fpc->getType()->getContext());
  return ConstantInt::get(intty, fixval, true);
}


Value *FloatToFixed::genConvertFixToFloat(Value *fix, Type *destt)
{
  Instruction *i = dyn_cast<Instruction>(fix);
  if (!i)
    return nullptr;
  FixToFloatCount++;

  IRBuilder<> builder(i->getNextNode());
  double twoebits = pow(2.0, fracBitsAmt);
  return builder.CreateFDiv(
    builder.CreateSIToFP(
      fix, destt),
    ConstantFP::get(destt, twoebits));
}


Type *FloatToFixed::getFixedPointType(LLVMContext &ctxt)
{
  return Type::getIntNTy(ctxt, bitsAmt);
}

