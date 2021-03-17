#include "TAFFOMath.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/IR/Type.h"
#include <llvm/Support/ErrorHandling.h>


using namespace flttofix;
using namespace llvm;
using namespace taffo;


namespace TaffoMath {

/** Create a new fixedpoint from current float
 * @param cont context used to create instruction
 * @param ref used to access private function
 * @param current_float float to be converted
 * @param match point used as match
 * @param outI instruction returned
 * @param outF fixedpoint returned *
 */
 bool createFixedPointFromConst(
    llvm::LLVMContext &cont, FloatToFixed *ref, const double &current_float,
    const FixedPointType &match, llvm::Constant *&outI, FixedPointType &outF

) {
  flttofix::FloatToFixed::TypeMatchPolicy typepol =
      flttofix::FloatToFixed::TypeMatchPolicy::ForceHint;
  APFloat val(current_float);

  APSInt fixval;

  APFloat tmp(val);

  bool precise = false;
  tmp.convert(APFloatBase::IEEEdouble(), APFloat::rmTowardNegative, &precise);

  double dblval = tmp.convertToDouble();
  int nbits = match.scalarBitsAmt();
  mdutils::Range range(dblval, dblval);
  int minflt = ref->isMaxIntPolicy(typepol) ? -1 : 0;
  mdutils::FPType t =
      taffo::fixedPointTypeFromRange(range, nullptr, nbits, minflt);
  outF = FixedPointType(&t);



  LLVM_DEBUG(dbgs() << "OutF: " << outF << " < Match:" << match <<"\n");
  if(outF.scalarFracBitsAmt() < match.scalarFracBitsAmt()){
    LLVM_DEBUG(dbgs() << "cannot insert " << current_float << " in "  << match << "\n");
    return false;
  }


  outF = FixedPointType(match);
  LLVM_DEBUG(dbgs() << "create fixed point from " << current_float << " to "
                    << outF << "\n");

  // create value
  APFloat exp(pow(2.0, outF.scalarFracBitsAmt()));

  exp.convert(val.getSemantics(), APFloat::rmTowardNegative, &precise);

  val.multiply(exp, APFloat::rmTowardNegative);

  fixval = APSInt(match.scalarBitsAmt(), !match.scalarIsSigned());

  APFloat::opStatus cvtres =
      val.convertToInteger(fixval, APFloat::rmTowardNegative, &precise);

  if (cvtres != APFloat::opStatus::opOK) {
    SmallVector<char, 64> valstr;
    val.toString(valstr);
    std::string valstr2(valstr.begin(), valstr.end());
    if (cvtres == APFloat::opStatus::opInexact) {
      LLVM_DEBUG(dbgs() << "ImpreciseConstConversion "
                        << "fixed point conversion of constant " << valstr2
                        << " is not precise\n");
    } else {

      LLVM_DEBUG(dbgs() << "ConstConversionFailed "
                        << " impossible to convert constant " << valstr2
                        << " to fixed point\n");
      return false;
    }
  }

  Type *intty = outF.scalarToLLVMType(cont);
  outI = ConstantInt::get(intty, fixval);
  LLVM_DEBUG(dbgs() << "create int "
                    << (dyn_cast<llvm::Constant>(outI)->dump(), "") << "from "
                    << current_float << "\n");
  return true;
}

Value *addAllocaToStart(FloatToFixed *ref, Function *newfs,
                        llvm::IRBuilder<> &builder, Type *to_alloca,
                        llvm::Value *ArraySize,
                        const llvm::Twine &Name) {
  auto OldBB = builder.GetInsertBlock();
  LLVM_DEBUG(dbgs() << "Insert alloca inst\nOld basic block: " << OldBB << "\n" );
  builder.SetInsertPoint(&(newfs->getEntryBlock()),
                         (newfs->getEntryBlock()).getFirstInsertionPt());
  Value *pointer_to_alloca = builder.CreateAlloca(to_alloca, ArraySize, Name);
  LLVM_DEBUG(dbgs() << "New Alloca: " << pointer_to_alloca << "\n" );
  builder.SetInsertPoint(OldBB);
  return pointer_to_alloca;
}

void getFixedFromRet(FloatToFixed *ref, Function *oldf,
                            FixedPointType &fxpret, bool &found) {
  for (auto i : oldf->users()) {
    if (isa<llvm::CallInst>(i)) {
      llvm::CallInst *callInst = dyn_cast<llvm::CallInst>(i);
      if (ref->hasInfo(callInst)) {
        fxpret = ref->valueInfo(callInst)->fixpType;
        LLVM_DEBUG(dbgs() << "\nReturn type: Scalar bits "
                          << fxpret.scalarBitsAmt() << "Fractional bits"
                          << fxpret.scalarFracBitsAmt() << "\n");
        found = true;
        return;
      }
    }
  }
  found = false;
}

void getFixedFromArg(FloatToFixed *ref, Function *oldf,
                            FixedPointType &fxparg, int n, bool &found) {
  for (auto arg = oldf->arg_begin(); arg != oldf->arg_end(); ++arg) {
    if (n == 0) {

      LLVM_DEBUG(dbgs() << "Try n: " << n << "\n");
      if (ref->hasInfo(arg)) {
        LLVM_DEBUG(dbgs() << "arg: "
                          << "\n");
        fxparg = ref->valueInfo(arg)->fixpType;
        LLVM_DEBUG(dbgs() << "\nReturn arg: Scalar bits "
                          << fxparg.scalarBitsAmt() << "Fractional bits"
                          << fxparg.scalarFracBitsAmt() << "\n");
        found = true;
        return;
      }
    }
    --n;
  }
  found = false;
}

llvm::GlobalVariable *
createGlobalConst(llvm::Module *module, llvm::StringRef Name, llvm::Type *Ty,
                  Constant *initializer, unsigned int alignment) {

  module->getOrInsertGlobal(Name, Ty);
  auto global = module->getGlobalVariable(Name);
  if(!global->hasInitializer()){
  global->setInitializer(initializer);
  global->setConstant(true);
  if (alignment > 1)
    global->setAlignment(alignment);
  }
  return global;
}

} // namespace TaffoMath


namespace flttofix {

void createStub(Function *OldFunc){
      auto BB = BasicBlock::Create(OldFunc->getContext(), OldFunc->getName() + ".entry", OldFunc);
      IRBuilder<> builder(BB, BB->getFirstInsertionPt());
      Value *V;
      if (OldFunc->getReturnType()->isFloatingPointTy()) {
        V = ConstantFP::get(OldFunc->getReturnType(), 0);
        builder.CreateRet(V);
        return;

      } else if (OldFunc->getReturnType()->isIntegerTy()) {
        V = ConstantInt::get(OldFunc->getReturnType(), 0);
        builder.CreateRet(V);
        return;
      }
      llvm_unreachable("return not handle");
}


void FloatToFixed::populateFunction(
    Function *NewFunc, Function *OldFunc, ValueToValueMapTy &VMap,
    bool ModuleLevelChanges, SmallVectorImpl<ReturnInst *> &Returns,
    const char *NameSuffix, ClonedCodeInfo *CodeInfo,
    ValueMapTypeRemapper *TypeMapper, ValueMaterializer *Materializer) {
  llvm::SmallVector<std::pair<unsigned int, llvm::MDNode *>, 10> savedMetadata;
  OldFunc->setLinkage(GlobalVariable::LinkageTypes::InternalLinkage);
  NewFunc->setLinkage(GlobalVariable::LinkageTypes::InternalLinkage);
  OldFunc->getAllMetadata(savedMetadata);
  // remove old body
  if (OldFunc->begin() != OldFunc->end()) {
    LLVM_DEBUG(dbgs() << "\ndelete body"
                      << "\n");
    OldFunc->deleteBody();
  }

  SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3> to_change;
  createStub(OldFunc);
    LLVM_DEBUG(dbgs() << "start clone"
                    << "\n");
  CloneFunctionInto(NewFunc, OldFunc, VMap, true, Returns);
    LLVM_DEBUG(dbgs() << "end clone"
                    << "\n");
  getFunctionInto(NewFunc, OldFunc, to_change);
  for (auto &MD : savedMetadata) {
    OldFunc->setMetadata(MD.first, MD.second);
    NewFunc->setMetadata(MD.first, MD.second);
  }
  
  LLVM_DEBUG(NewFunc->getParent()->dump());
  LLVM_DEBUG(llvm::verifyFunction(*NewFunc, &dbgs()));
}

/*
  bool FloatToFixed::createAbs(
    llvm::Function *newfs, llvm::Function *oldf,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change){
        llvm::LLVMContext &cont(oldf->getContext());
        DataLayout dataLayout(oldf->getParent());
        LLVM_DEBUG(dbgs() << "\nGet Context " << &cont << "\n");
        // get first basick block of function
        BasicBlock::Create(cont, "Entry", oldf);
        BasicBlock *where = &(oldf->getEntryBlock());
        LLVM_DEBUG(dbgs() << "\nGet entry point " << where);
        IRBuilder<> builder(where, where->getFirstInsertionPt());
        // get return type fixed point
        flttofix::FixedPointType fxpret;
        flttofix::FixedPointType fxparg;
        bool foundRet = false;
        bool foundArg = false;
        TaffoMath::getFixedFromRet(this, oldf, fxpret, foundRet);
        // get argument fixed point
        TaffoMath::getFixedFromArg(this, newfs, fxparg, 0, foundArg);

        if(foundRet && foundArg){
          llvm::Instruction *inst = nullptr;
          llvm::Type *arg_type = fxparg.scalarToLLVMType(cont);
          inst = builder.CreateAlloca(arg_type);
          to_change.push_back({inst->getParent(), {inst}});
          inst = builder.CreateStore(builder.CreateAnd(builder.CreateLoad(inst), builder.CreateNot(builder.CreateShl(llvm::ConstantInt::get(arg_type,1),arg_type->getScalarSizeInBits()-1))),inst);
          to_change.push_back({inst->getParent(), {inst}});
          if (oldf->getReturnType()->isFloatingPointTy()) {
            builder.CreateRet(ConstantFP::get(oldf->getReturnType(), 0.0f));
          } else {
            builder.CreateRet(ConstantInt::get(oldf->getReturnType(), 0));
          }
        } else {
          if (oldf->getReturnType()->isFloatingPointTy()) {
            builder.CreateRet(ConstantFP::get(oldf->getReturnType(), 0.0f));
          } else {
            builder.CreateRet(ConstantInt::get(oldf->getReturnType(), 0));
          }
          // llvm_unreachable("Abs not transformed");
        }
        }
*/

bool partialSpecialCall(
    llvm::Function *newf, bool &foundRet, flttofix::FixedPointType &fxpret) {
  Module *m = newf->getParent();
  StringRef fName = newf->getName();
  BasicBlock *where = &(newf->getEntryBlock());
  IRBuilder<> builder(where, where->getFirstInsertionPt());
  Value *generic;
  Value *ret;
  auto end = BasicBlock::Create(m->getContext(), "end", newf);
  auto trap = Intrinsic::getDeclaration(m, Intrinsic::trap);
  builder.CreateCall(trap);
  builder.CreateBr(end);
  builder.SetInsertPoint(end);
  if(newf->getReturnType()->isFloatingPointTy()){
  builder.CreateRet(ConstantFP::get(newf->getReturnType(), 1212.0f));
  }else{
    builder.CreateRet(ConstantInt::get(newf->getReturnType(), 1212));
  }
  LLVM_DEBUG(dbgs() << "Not handled\n");
  return false;
}



bool FloatToFixed::createAbs(llvm::Function *newfs, llvm::Function *oldf){
  llvm::LLVMContext &cont(oldf->getContext());
  DataLayout dataLayout(oldf->getParent());
  LLVM_DEBUG(dbgs() << "\nGet Context " << &cont << "\n");
  // get first basick block of function
  auto arg_type = newfs->getArg(0)->getType();
  auto ret_type = newfs->getReturnType();
  BasicBlock *where = &(newfs->getEntryBlock());
    LLVM_DEBUG(dbgs() << "\nGet entry point " << where);
    IRBuilder<> builder(where, where->getFirstInsertionPt());
    if (this->hasInfo(oldf->getArg(0)) && !(this->valueInfo(oldf->getArg(0))->fixpType.scalarIsSigned())) {      
      builder.CreateRet(newfs->getArg(0));
      return true;
    }
    LLVM_DEBUG(dbgs() << "\nGet insertion\n");
    auto* inst = builder.CreateBitCast(newfs->getArg(0), llvm::Type::getIntNTy(cont, arg_type->getPrimitiveSizeInBits()));
    LLVM_DEBUG(dbgs() << "\nGet bitcast\n");
    inst = builder.CreateSelect( builder.CreateICmpEQ(builder.CreateLShr(inst,arg_type->getScalarSizeInBits()-1), llvm::ConstantInt::get(llvm::Type::getIntNTy(cont, arg_type->getPrimitiveSizeInBits()), 1)), builder.CreateBitCast(
        builder.CreateSub( llvm::ConstantInt::get(llvm::Type::getIntNTy(cont, arg_type->getPrimitiveSizeInBits()), 0), inst), arg_type ) , inst);


    LLVM_DEBUG(dbgs() << "\nType ret"<< (ret_type->dump()," ") << "\n");
    LLVM_DEBUG(dbgs() << "\nType arg" << (arg_type->dump(), " ") <<"\n");

    if (arg_type->isFloatingPointTy() && ret_type->isFloatingPointTy()) {
      inst = builder.CreateFPCast(inst, ret_type);
    } else if (arg_type->isFloatingPointTy() && ret_type->isIntegerTy()) {
      inst = builder.CreateFPToSI(inst, ret_type);
    } else if (arg_type->isIntegerTy() && ret_type->isFloatingPointTy()) {
      inst = builder.CreateSIToFP(inst, ret_type);
    } else if (arg_type->isIntegerTy() && ret_type->isIntegerTy()) {
      inst = builder.CreateIntCast(inst, ret_type, true);
    }
    builder.CreateRet(inst);


}


bool FloatToFixed::getFunctionInto(
    Function *NewFunc, Function *OldFunc,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  auto fName = HandledFunction::demangle(OldFunc->getName());


  if (taffo::start_with(fName, "sin") || taffo::start_with(fName, "cos") ||
      taffo::start_with(fName, "_ZSt3cos") ||
      taffo::start_with(fName, "_ZSt3sin")) {
    return createSinCos(NewFunc, OldFunc);
  
  }

  if(taffo::start_with(fName, "asin"))
  {
    return createASin(NewFunc, OldFunc);
  }

  if(taffo::start_with(fName, "acos"))
  {
    return createACos(NewFunc, OldFunc);
  }


  if(taffo::start_with(fName, "abs")){
    return createAbs(NewFunc, OldFunc);
  }
  llvm_unreachable("Function not recognized");
}

} // namespace flttofix