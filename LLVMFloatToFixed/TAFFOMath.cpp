#include "TAFFOMath.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Verifier.h"
#include "string"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace flttofix;
using namespace llvm;
using namespace taffo;

namespace TaffoMathConstant {

/** Create a new fixedpoint from current float
 * @param cont context used to create instruction
 * @param ref used to access private function
 * @param current_float float to be converted
 * @param match point used as match
 * @param outI instruction returned
 * @param outF fixedpoint returned *
 */
inline void createFixedPointFromConst(
    llvm::LLVMContext &cont, FloatToFixed *ref, const float &current_float,
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

  assert(match.scalarBitsAmt() == outF.scalarBitsAmt() &&
         outF.scalarFracBitsAmt() <= match.scalarBitsAmt());

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
      assert(false && "ConstConversionFailed forbidden...\n");
    }
  }

  Type *intty = outF.scalarToLLVMType(cont);
  outI = ConstantInt::get(intty, fixval);
  LLVM_DEBUG(dbgs() << "create int "
                    << (dyn_cast<llvm::Constant>(outI)->dump(), "") << "from "
                    << current_float << "\n");
}
} // namespace TaffoMathConstant

/**
 * @param ref used to access member function
 * @param oldf function used to take ret information
 * @param fxpret output of the function *
 * */
inline void getFixedFromRet(FloatToFixed *ref, Function *oldf,
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

inline void getFixedFromArg(FloatToFixed *ref, Function *oldf,
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

inline llvm::GlobalVariable *
createGlobalConst(llvm::Module *module, llvm::StringRef Name, llvm::Type *Ty,
                  Constant *initializer, unsigned int alignment = 1) {

  module->getOrInsertGlobal(Name, Ty);
  auto global = module->getGlobalVariable(Name);
  global->setInitializer(initializer);
  global->setConstant(true);
  if (alignment > 1)
    global->setAlignment(alignment);
  return global;
}

namespace flttofix {

bool partialSpecialCallSinCos(
    llvm::Function *oldf, bool &foundRet, flttofix::FixedPointType &fxpret,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {
  Module *m = oldf->getParent();
  StringRef fName = oldf->getName();
  BasicBlock *where = &(oldf->getEntryBlock());
  IRBuilder<> builder(where, where->getFirstInsertionPt());
  Value *generic;
  Value *ret;
  if (foundRet) {
    auto int_type = fxpret.scalarToLLVMType(oldf->getContext());
    ret = builder.CreateAlloca(int_type, nullptr);
    builder.CreateStore(ConstantInt::get(int_type, 1212), ret);
    ret = builder.CreateLoad(ret);
  } else {
    auto float_type = oldf->getReturnType();
    ret = builder.CreateAlloca(float_type, nullptr);
    builder.CreateStore(ConstantFP::get(float_type, 1212.0), ret);
    ret = builder.CreateLoad(ret);
  }
  auto end = BasicBlock::Create(m->getContext(), "end", oldf);
  auto trap = Intrinsic::getDeclaration(m, Intrinsic::trap);
  builder.CreateCall(trap);
  builder.CreateBr(end);
  builder.SetInsertPoint(end);
  builder.CreateRet(ConstantFP::get(oldf->getReturnType(), 1212.0f));
  to_change.push_back({end, {ret}});
  LLVM_DEBUG(dbgs() << "Not handled\n");
  return false;
}

Value *addAllocaToStart(FloatToFixed *ref, Function *oldf,
                        llvm::IRBuilder<> &builder, Type *to_alloca,
                        llvm::Value *ArraySize = (llvm::Value *)nullptr,
                        const llvm::Twine &Name = "") {
  auto OldBB = builder.GetInsertBlock();
  LLVM_DEBUG(dbgs() << "Insert alloca inst\nOld basic block: " << OldBB << "\n" );
  builder.SetInsertPoint(&(oldf->getEntryBlock()),
                         (oldf->getEntryBlock()).getFirstInsertionPt());
  Value *pointer_to_alloca = builder.CreateAlloca(to_alloca, ArraySize, Name);
  LLVM_DEBUG(dbgs() << "New Alloca: " << pointer_to_alloca << "\n" );
  builder.SetInsertPoint(OldBB);
  return pointer_to_alloca;
}

void fixrangeSinCos(FloatToFixed *ref, Function *oldf, FixedPointType &fxparg,
                    FixedPointType &fxpret, Value *arg_value,
                    llvm::IRBuilder<> &builder) {
  assert(fxparg.scalarBitsAmt() == fxpret.scalarBitsAmt() &&
         "different type arg and ret");
  int int_lenght = fxparg.scalarBitsAmt();
  Module *m = oldf->getParent();
  llvm::LLVMContext &cont = oldf->getContext();
  DataLayout dataLayout(oldf->getParent());
  auto int_type = fxparg.scalarToLLVMType(cont);
  Value *generic = nullptr;
  int max = fxparg.scalarFracBitsAmt() > fxpret.scalarFracBitsAmt()
                ? fxparg.scalarFracBitsAmt()
                : fxpret.scalarFracBitsAmt();
  int min = fxparg.scalarFracBitsAmt() < fxpret.scalarFracBitsAmt()
                ? fxparg.scalarFracBitsAmt()
                : fxpret.scalarFracBitsAmt();
  LLVM_DEBUG(dbgs() << "Max: " << max << " Min: " << min << "\n"); 
  // create pi_2 table
  TaffoMathConstant::pair_ftp_value<llvm::Constant *, 5> pi_2_vect;
  for (int i = 0; i <= max - min; ++i) {
    pi_2_vect.fpt.push_back(
        flttofix::FixedPointType(fxparg.scalarIsSigned(), min + i, int_lenght));
    Constant *tmp = nullptr;
    flttofix::FixedPointType match = flttofix::FixedPointType(fxparg.scalarIsSigned(), min + i, int_lenght);
    auto &current_fpt = pi_2_vect.fpt.front();
    TaffoMathConstant::createFixedPointFromConst(
        cont, ref, TaffoMathConstant::pi_2, match, tmp, current_fpt);
    pi_2_vect.value.push_back(tmp);
  }
  auto pi_2_ArrayType =
      llvm::ArrayType::get(pi_2_vect.value.front()->getType(), max - min + 1);
  LLVM_DEBUG(dbgs() << "ArrayType  " << pi_2_ArrayType << "\n");
  auto pi_2_ConstArray = llvm::ConstantArray::get(
      pi_2_ArrayType, llvm::ArrayRef<llvm::Constant *>(pi_2_vect.value));
  LLVM_DEBUG(dbgs() << "ConstantDataArray pi_2 " << pi_2_ConstArray << "\n");
  auto alignement_pi_2 =
      dataLayout.getPrefTypeAlignment(pi_2_vect.value.front()->getType());
  LLVM_DEBUG(dbgs() << (pi_2_ArrayType->dump(), pi_2_ConstArray->dump(), "alignment: ") <<  alignement_pi_2 << "\n");
  auto pi_2_arry_g =
      createGlobalConst(oldf->getParent(), "pi_2_global." + std::to_string(min) + "_" + std::to_string(max), pi_2_ArrayType,
                        pi_2_ConstArray, alignement_pi_2);
  auto pointer_to_array = addAllocaToStart(ref, oldf, builder, pi_2_ArrayType,
                                           nullptr, "pi_2_array");
 dyn_cast<llvm::AllocaInst>(pointer_to_array)->setAlignment(alignement_pi_2);
  builder.CreateMemCpy(
      pointer_to_array, alignement_pi_2, pi_2_arry_g, alignement_pi_2,
      (max-min+1) * (int_type->getScalarSizeInBits() >> 3));
  LLVM_DEBUG(dbgs() << "\nAdd pi_2 table"
                    << "\n");

  int current_arg_point = fxparg.scalarFracBitsAmt();
  int current_ret_point = fxpret.scalarFracBitsAmt();

  Value *iterator_pi_2 =
      addAllocaToStart(ref, oldf, builder, int_type, nullptr, "Iterator_pi_2");
  Value *point_arg =
      addAllocaToStart(ref, oldf, builder, int_type, nullptr, "point_arg");
  Value *point_ret =
      addAllocaToStart(ref, oldf, builder, int_type, nullptr, "point_ret");

  builder.CreateStore(ConstantInt::get(int_type, current_arg_point), point_arg);
  builder.CreateStore(ConstantInt::get(int_type, current_ret_point), point_ret);

  if (current_arg_point < current_ret_point) {
    BasicBlock *normalize_cond =
        BasicBlock::Create(cont, "normalize_cond", oldf);
    BasicBlock *normalize = BasicBlock::Create(cont, "normalize", oldf);
    BasicBlock *cmp_bigger_than_2pi =
        BasicBlock::Create(cont, "cmp_bigger_than_2pi", oldf);
    BasicBlock *bigger_than_2pi =
        BasicBlock::Create(cont, "bigger_than_2pi", oldf);
    BasicBlock *end_loop = BasicBlock::Create(cont, "body", oldf);
    builder.CreateStore(ConstantInt::get(int_type, 0), iterator_pi_2);
    builder.CreateBr(normalize_cond);
    LLVM_DEBUG(dbgs() << "add Normalize\n");
    // normalize cond
    builder.SetInsertPoint(normalize_cond);
    Value *last_bit_mask =
        fxparg.scalarIsSigned()
            ? ConstantInt::get(int_type, 1 << (int_lenght - 2))
            : ConstantInt::get(int_type, 1 << (int_lenght - 1));
    generic = builder.CreateAnd(builder.CreateLoad(arg_value), last_bit_mask);
    generic = builder.CreateAnd(
        builder.CreateICmpEQ(generic, ConstantInt::get(int_type, 0)),
        builder.CreateICmpSLT(builder.CreateLoad(point_arg),
                              builder.CreateLoad(point_ret)));
    builder.CreateCondBr(generic, normalize, cmp_bigger_than_2pi);
    // normalize
    builder.SetInsertPoint(normalize);
    builder.CreateStore(builder.CreateShl(builder.CreateLoad(arg_value),
                                          ConstantInt::get(int_type, 1)),
                        arg_value);
    builder.CreateStore(builder.CreateAdd(builder.CreateLoad(iterator_pi_2),
                                          ConstantInt::get(int_type, 1)),
                        iterator_pi_2);
    builder.CreateStore(builder.CreateAdd(builder.CreateLoad(point_arg),
                                          ConstantInt::get(int_type, 1)),
                        point_arg);
    builder.CreateBr(normalize_cond);
    LLVM_DEBUG(dbgs() << "add bigger than 2pi\n");
    // cmp_bigger_than_2pi
    builder.SetInsertPoint(cmp_bigger_than_2pi);
    generic = builder.CreateGEP(
        pointer_to_array,
        {ConstantInt::get(int_type, 0), builder.CreateLoad(iterator_pi_2)});
    generic = builder.CreateLoad(generic);
    builder.CreateCondBr(builder.CreateICmpSLE(generic,
                                               builder.CreateLoad(arg_value)),
                         bigger_than_2pi, end_loop);
    // bigger_than_2pi
    builder.SetInsertPoint(bigger_than_2pi);
    builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                          generic),
                        arg_value);
    builder.CreateBr(normalize_cond);
    builder.SetInsertPoint(end_loop);
  } else {
    LLVM_DEBUG(dbgs() << "add bigger than 2pi\n");
    BasicBlock *cmp_bigger_than_2pi =
        BasicBlock::Create(cont, "cmp_bigger_than_2pi", oldf);
    BasicBlock *bigger_than_2pi =
        BasicBlock::Create(cont, "bigger_than_2pi", oldf);
    BasicBlock *body = BasicBlock::Create(cont, "shift", oldf);
    builder.CreateStore(ConstantInt::get(int_type, 0), iterator_pi_2);
    builder.CreateBr(cmp_bigger_than_2pi);
    builder.SetInsertPoint(cmp_bigger_than_2pi);
    generic = builder.CreateGEP(
        pointer_to_array,
        {ConstantInt::get(int_type, 0), builder.CreateLoad(iterator_pi_2)});
    generic = builder.CreateLoad(generic);
    builder.CreateCondBr(builder.CreateICmpSLE(generic,
                                               builder.CreateLoad(arg_value)),
                         bigger_than_2pi, body);
    builder.SetInsertPoint(bigger_than_2pi);
    builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                          generic),
                        arg_value);
    builder.CreateBr(cmp_bigger_than_2pi);
    builder.SetInsertPoint(body);
  }
  // set point at same position
  {
    LLVM_DEBUG(dbgs() << "set point at same position\n");
    builder.CreateStore(ConstantInt::get(int_type, 0), iterator_pi_2);
    BasicBlock *body_2 = BasicBlock::Create(cont, "body", oldf);
    BasicBlock *true_block = BasicBlock::Create(cont, "left_shift", oldf);
    BasicBlock *false_block = BasicBlock::Create(cont, "right_shift", oldf);   
    
    BasicBlock *end = BasicBlock::Create(cont, "body", oldf);
    builder.CreateCondBr(builder.CreateICmpEQ(builder.CreateLoad(point_arg),
                                              builder.CreateLoad(point_ret)),
                         end, body_2);
    builder.SetInsertPoint(body_2);
    builder.CreateCondBr(builder.CreateICmpSLT(builder.CreateLoad(point_arg),
                                               builder.CreateLoad(point_ret)),
                         true_block, false_block);
    builder.SetInsertPoint(true_block);
    generic = builder.CreateSub(builder.CreateLoad(point_ret),
                                builder.CreateLoad(point_arg));
    builder.CreateStore(
        builder.CreateShl(builder.CreateLoad(arg_value), generic), arg_value);
    builder.CreateBr(end);
    builder.SetInsertPoint(false_block);
    generic = builder.CreateSub(builder.CreateLoad(point_arg),
                                builder.CreateLoad(point_ret));
    builder.CreateStore(
        builder.CreateAShr(builder.CreateLoad(arg_value), generic), arg_value);
    builder.CreateBr(end);
    builder.SetInsertPoint(end);
  }
    LLVM_DEBUG(dbgs() << "End fixrange\n");
}


bool FloatToFixed::createSinCos(
    llvm::Function *newfs, llvm::Function *oldf,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  //
  Value *generic;
  bool isSin =
      oldf->getName().find("sin") == 0 || oldf->getName().find("_ZSt3sin") == 0;
  LLVM_DEBUG(dbgs() << "is sin: " << isSin << "\n");
  // retrive context used in later instruction
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
  getFixedFromRet(this, oldf, fxpret, foundRet);
  // get argument fixed point
  getFixedFromArg(this, oldf, fxparg, 0, foundArg);
  if (!foundRet || !foundArg) {
    return partialSpecialCallSinCos(oldf, foundRet, fxpret, to_change);
  }
  TaffoMathConstant::pair_ftp_value<llvm::Value *> arg(fxparg);
  arg.value = oldf->arg_begin();
  auto truefxpret = fxpret;
  LLVM_DEBUG(dbgs() << "fxpret: " << fxpret.scalarBitsAmt() <<  " frac part: " << fxpret.scalarFracBitsAmt() << " difference: " << fxpret.scalarBitsAmt() - fxpret.scalarFracBitsAmt() <<"\n" );
  if ((fxpret.scalarBitsAmt() - fxpret.scalarFracBitsAmt()) < 4) {    
      fxpret = flttofix::FixedPointType(true,
                                        fxpret.scalarBitsAmt() - 4,
                                        fxpret.scalarBitsAmt());
      LLVM_DEBUG(dbgs() << "New fxpret: " << fxpret << "\n");    
  }


  auto int_type = fxpret.scalarToLLVMType(cont);
  auto internal_fxpt = flttofix::FixedPointType(true, fxpret.scalarBitsAmt()-2, fxpret.scalarBitsAmt() );
  // create local variable
  TaffoMathConstant::pair_ftp_value<llvm::Value *> x_value(internal_fxpt);
  TaffoMathConstant::pair_ftp_value<llvm::Value *> y_value(internal_fxpt);
  x_value.value = builder.CreateAlloca(int_type, nullptr, "x");
  y_value.value = builder.CreateAlloca(int_type, nullptr, "y");
  auto changeSign =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "changeSign");
  auto changedFunction =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "changefunc");
  auto specialAngle =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "specialAngle");
  Value *arg_value = builder.CreateAlloca(int_type, nullptr, "arg");
  Value *i_iterator = builder.CreateAlloca(int_type, nullptr, "iterator");

  // create pi variable
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_2(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_32(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_half(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> kopp(internal_fxpt);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> zero(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> zeroarg(fxparg);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> zero_internal(internal_fxpt);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> one(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> one_internal(internal_fxpt);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> minus_one(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> minus_one_internal(internal_fxpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::pi, fxpret, pi.value, pi.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::pi_2, fxpret, pi_2.value, pi_2.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::pi_32, fxpret, pi_32.value, pi_32.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::pi_half, fxpret, pi_half.value,
      pi_half.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::Kopp, internal_fxpt, kopp.value, kopp.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::zero, fxpret, zero.value, zero.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::zero, fxparg, zeroarg.value, zeroarg.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::one, fxpret, one.value, one.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::one, internal_fxpt, one_internal.value, one_internal.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::zero, internal_fxpt, zero_internal.value, zero_internal.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::minus_one, fxpret, minus_one.value,
      minus_one.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::minus_one, internal_fxpt, minus_one_internal.value,
      minus_one_internal.fpt);
      std::string S_ret_point = "." + std::to_string(fxpret.scalarFracBitsAmt());



  pi.value = createGlobalConst(
      oldf->getParent(), "pi" + S_ret_point, pi.fpt.scalarToLLVMType(cont), pi.value,
      dataLayout.getPrefTypeAlignment(pi.fpt.scalarToLLVMType(cont)));
  pi_2.value = createGlobalConst(
      oldf->getParent(), "pi_2" + S_ret_point, pi_2.fpt.scalarToLLVMType(cont), pi_2.value,
      dataLayout.getPrefTypeAlignment(pi_2.fpt.scalarToLLVMType(cont)));
  pi_32.value = createGlobalConst(
      oldf->getParent(), "pi_32" + S_ret_point, pi_32.fpt.scalarToLLVMType(cont), pi_32.value,
      dataLayout.getPrefTypeAlignment(pi_32.fpt.scalarToLLVMType(cont)));
  pi_half.value = createGlobalConst(
      oldf->getParent(), "pi_half"+ S_ret_point, pi_half.fpt.scalarToLLVMType(cont),
      pi_half.value,
      dataLayout.getPrefTypeAlignment(pi_half.fpt.scalarToLLVMType(cont)));
  kopp.value = createGlobalConst(
      oldf->getParent(), "kopp" + S_ret_point, kopp.fpt.scalarToLLVMType(cont), kopp.value,
      dataLayout.getPrefTypeAlignment(kopp.fpt.scalarToLLVMType(cont)));
  zero.value = createGlobalConst(
      oldf->getParent(), "zero" + S_ret_point, zero.fpt.scalarToLLVMType(cont), zero.value,
      dataLayout.getPrefTypeAlignment(zero.fpt.scalarToLLVMType(cont)));
  zeroarg.value = createGlobalConst(
      oldf->getParent(), "zero_arg" + S_ret_point, zeroarg.fpt.scalarToLLVMType(cont),
      zeroarg.value,
      dataLayout.getPrefTypeAlignment(zeroarg.fpt.scalarToLLVMType(cont)));
  one.value = createGlobalConst(
      oldf->getParent(), "one" + S_ret_point, one.fpt.scalarToLLVMType(cont), one.value,
      dataLayout.getPrefTypeAlignment(one.fpt.scalarToLLVMType(cont)));
  minus_one.value = createGlobalConst(
      oldf->getParent(), "minus_one" + S_ret_point, minus_one.fpt.scalarToLLVMType(cont),
      minus_one.value,
      dataLayout.getPrefTypeAlignment(minus_one.fpt.scalarToLLVMType(cont)));
    minus_one_internal.value = createGlobalConst(
      oldf->getParent(), "minus_one_internal." + std::to_string(internal_fxpt.scalarFracBitsAmt()), minus_one_internal.fpt.scalarToLLVMType(cont),
      minus_one_internal.value,
      dataLayout.getPrefTypeAlignment(minus_one.fpt.scalarToLLVMType(cont)));
    one_internal.value = createGlobalConst(
      oldf->getParent(), "one_internal." + std::to_string(internal_fxpt.scalarFracBitsAmt()), one_internal.fpt.scalarToLLVMType(cont),
      one_internal.value,
      dataLayout.getPrefTypeAlignment(minus_one.fpt.scalarToLLVMType(cont)));
  
  /** create arctan table
   **/
  LLVM_DEBUG(dbgs() << "Create arctan table"
                    << "\n");
  TaffoMathConstant::pair_ftp_value<llvm::Constant *,
                                    TaffoMathConstant::TABLELENGHT>
      arctan_2power;
  for (int i = 0; i < TaffoMathConstant::TABLELENGHT; i++) {
        arctan_2power.fpt.push_back(flttofix::FixedPointType(fxpret));
    Constant *tmp = nullptr;
    auto &current_fpt = arctan_2power.fpt.front();
    TaffoMathConstant::createFixedPointFromConst(
        cont, this, TaffoMathConstant::arctan_2power[i], internal_fxpt ,  tmp, current_fpt);
    arctan_2power.value.push_back(tmp);
    LLVM_DEBUG(dbgs() << i << ")");
  }

  auto arctanArrayType = llvm::ArrayType::get(arctan_2power.value[0]->getType(),
                                              TaffoMathConstant::TABLELENGHT);
  LLVM_DEBUG(dbgs() << "ArrayType  " << arctanArrayType << "\n");
  auto arctanConstArray = llvm::ConstantArray::get(
      arctanArrayType, llvm::ArrayRef<llvm::Constant *>(arctan_2power.value));
  LLVM_DEBUG(dbgs() << "ConstantDataArray tmp2 " << arctanConstArray << "\n");
  auto alignement_arctan =
      dataLayout.getPrefTypeAlignment(arctan_2power.value[0]->getType());
  auto arctan_g =
      createGlobalConst(oldf->getParent(), "arctan_g." + std::to_string(internal_fxpt.scalarFracBitsAmt()), arctanArrayType,
                        arctanConstArray, alignement_arctan);

  auto pointer_to_array = builder.CreateAlloca(arctanArrayType);
  pointer_to_array->setAlignment(alignement_arctan);

  builder.CreateMemCpy(
      pointer_to_array, alignement_arctan, arctan_g, alignement_arctan,
      TaffoMathConstant::TABLELENGHT * (int_type->getScalarSizeInBits() >> 3));
  LLVM_DEBUG(dbgs() << "\nAdd to newf arctan table"
                    << "\n");

  // code gen
  auto int_8_zero = ConstantInt::get(Type::getInt8Ty(cont), 0);
  auto int_8_one = ConstantInt::get(Type::getInt8Ty(cont), 1);
  auto int_8_minus_one = ConstantInt::get(Type::getInt8Ty(cont), -1);
  builder.CreateStore(int_8_zero, changeSign);
  builder.CreateStore(int_8_zero, changedFunction);
  builder.CreateStore(int_8_zero, specialAngle);
  BasicBlock *to_remove = BasicBlock::Create(cont, "to_remove", oldf);
  builder.CreateBr(to_remove);
  builder.SetInsertPoint(to_remove);
  auto to_rem = builder.CreateFPToSI(arg.value, arg.fpt.scalarToLLVMType(cont));
  auto gen = builder.CreateStore(to_rem, arg_value);

  BasicBlock *body = BasicBlock::Create(cont, "body", oldf);
  builder.CreateBr(body);
  builder.SetInsertPoint(body);
  to_change.push_back({to_remove, {arg.value, arg_value, body}});
  arg.value = arg_value;
  BasicBlock *return_point = BasicBlock::Create(cont, "return_point", oldf);
  // handle unsigned arg
  if(!fxparg.scalarIsSigned()){
    builder.CreateStore(builder.CreateLShr(builder.CreateLoad(arg_value), ConstantInt::get(int_type, 1)),arg_value);
    fxparg.scalarFracBitsAmt() = fxparg.scalarFracBitsAmt() - 1;
    fxparg.scalarIsSigned()=true;
  }

  //handle too small arg
  {
    int diff =fxparg.scalarBitsAmt()- fxparg.scalarFracBitsAmt();
  if(diff < 4){
    builder.CreateStore(builder.CreateAShr(builder.CreateLoad(arg_value), ConstantInt::get(int_type, 4-diff)),arg_value);
    fxparg.scalarFracBitsAmt() = fxparg.scalarFracBitsAmt() - (4-diff);
    fxparg.scalarIsSigned()=true;
  }
  }
  
  // handle negative
  if (isSin) {
    // sin(-x) == -sin(x)
    {
      BasicBlock *true_greater_zero =
          BasicBlock::Create(cont, "true_greater_zero", oldf);
      BasicBlock *false_greater_zero = BasicBlock::Create(cont, "body", oldf);

      generic = builder.CreateICmpSLT(builder.CreateLoad(arg_value, "arg"),
                                      builder.CreateLoad(zeroarg.value));
      generic =
          builder.CreateCondBr(generic, true_greater_zero, false_greater_zero);
      builder.SetInsertPoint(true_greater_zero);
      generic = builder.CreateSub(builder.CreateLoad(zeroarg.value),
                                  builder.CreateLoad(arg_value));
      builder.CreateStore(
          builder.CreateXor(builder.CreateLoad(changeSign), int_8_minus_one),
          changeSign);
      builder.CreateStore(generic, arg_value);
      builder.CreateBr(false_greater_zero);
      builder.SetInsertPoint(false_greater_zero);
    }

  } else {
    // cos(-x) == cos(x)
    {
      BasicBlock *true_greater_zero =
          BasicBlock::Create(cont, "true_greater_zero", oldf);
      BasicBlock *false_greater_zero = BasicBlock::Create(cont, "body", oldf);

      generic = builder.CreateICmpSLT(builder.CreateLoad(arg_value, "arg"),
                                      builder.CreateLoad(zeroarg.value));
      generic =
          builder.CreateCondBr(generic, true_greater_zero, false_greater_zero);
      builder.SetInsertPoint(true_greater_zero);
      generic = builder.CreateSub(builder.CreateLoad(zeroarg.value),
                                  builder.CreateLoad(arg_value));
      builder.CreateStore(generic, arg_value);
      builder.CreateBr(false_greater_zero);
      builder.SetInsertPoint(false_greater_zero);
    }
  }

  fixrangeSinCos(this,oldf,fxparg,fxpret,arg_value,builder);

  // special case
  {
    // x = pi/2

    {
      BasicBlock *BTrue = BasicBlock::Create(cont, "equal_pi_2", oldf);
      BasicBlock *BFalse = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpEQ(builder.CreateLoad(arg_value, "arg"),
                                     builder.CreateLoad(pi_half.value));
      builder.CreateCondBr(generic, BTrue, BFalse);
      builder.SetInsertPoint(BTrue);
      builder.CreateStore(int_8_one, specialAngle);
      builder.CreateStore(builder.CreateLoad(one_internal.value), y_value.value);
      builder.CreateStore(builder.CreateLoad(zero.value), x_value.value);
      builder.CreateBr(BFalse);
      builder.SetInsertPoint(BFalse);
    }
    // x= pi

    {
      BasicBlock *BTrue = BasicBlock::Create(cont, "equal_pi", oldf);
      BasicBlock *BFalse = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpEQ(builder.CreateLoad(arg_value, "arg"),
                                     builder.CreateLoad(pi.value));
      builder.CreateCondBr(generic, BTrue, BFalse);
      builder.SetInsertPoint(BTrue);
      builder.CreateStore(int_8_one, specialAngle);
      builder.CreateStore(builder.CreateLoad(zero.value), y_value.value);
      builder.CreateStore(builder.CreateLoad(minus_one_internal.value), x_value.value);
      builder.CreateBr(BFalse);
      builder.SetInsertPoint(BFalse);
    }
    // x = pi_32

    {
      BasicBlock *BTrue = BasicBlock::Create(cont, "equal_pi_32", oldf);
      BasicBlock *BFalse = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpEQ(builder.CreateLoad(arg_value, "arg"),
                                     builder.CreateLoad(pi_32.value));
      builder.CreateCondBr(generic, BTrue, BFalse);
      builder.SetInsertPoint(BTrue);
      builder.CreateStore(int_8_one, specialAngle);
      builder.CreateStore(builder.CreateLoad(minus_one_internal.value), y_value.value);
      builder.CreateStore(builder.CreateLoad(zero.value), x_value.value);
      builder.CreateBr(BFalse);
      builder.SetInsertPoint(BFalse);
    }
    // x = 0

    {
      BasicBlock *BTrue = BasicBlock::Create(cont, "equal_0", oldf);
      BasicBlock *BFalse = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpEQ(builder.CreateLoad(arg_value, "arg"),
                                     builder.CreateLoad(zero.value));
      builder.CreateCondBr(generic, BTrue, BFalse);
      builder.SetInsertPoint(BTrue);
      builder.CreateStore(int_8_one, specialAngle);
      builder.CreateStore(builder.CreateLoad(zero.value), y_value.value);
      builder.CreateStore(builder.CreateLoad(one_internal.value), x_value.value);
      builder.CreateBr(BFalse);
      builder.SetInsertPoint(BFalse);
    }
    // handle premature return
    {
      BasicBlock *BTrue = BasicBlock::Create(cont, "Special", oldf);
      BasicBlock *BFalse = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpEQ(builder.CreateLoad(specialAngle, "arg"),
                                     int_8_one);
      builder.CreateCondBr(generic, BTrue, BFalse);
      builder.SetInsertPoint(BTrue);
      builder.CreateBr(return_point);
      builder.SetInsertPoint(BFalse);
    }
  }

  if (isSin) {
    // angle > pi_half && angle < pi sin(x) = cos(x - pi_half)
    {
      BasicBlock *in_II_quad = BasicBlock::Create(cont, "in_II_quad", oldf);
      BasicBlock *not_in_II_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(
          builder.CreateLoad(pi_half.value, "pi_half"),
          builder.CreateLoad(arg_value), "arg_greater_pi_half");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value), builder.CreateLoad(pi.value, "pi"),
          "arg_less_pi");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_II_quad, not_in_II_quad);
      builder.SetInsertPoint(in_II_quad);
      builder.CreateStore(builder.CreateXor(builder.CreateLoad(changedFunction),
                                            int_8_minus_one),
                          changedFunction);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi_half.value)),
                          arg_value);
      builder.CreateBr(not_in_II_quad);
      builder.SetInsertPoint(not_in_II_quad);
    }
    // angle > pi&& angle < pi_32(x) sin(x) = -sin(x - pi)
    {
      BasicBlock *in_III_quad = BasicBlock::Create(cont, "in_III_quad", oldf);
      BasicBlock *not_in_III_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(builder.CreateLoad(pi.value, "pi"),
                                      builder.CreateLoad(arg_value),
                                      "arg_greater_pi");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value),
          builder.CreateLoad(pi_32.value, "pi_32"), "arg_less_pi_32");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_III_quad, not_in_III_quad);
      builder.SetInsertPoint(in_III_quad);
      builder.CreateStore(
          builder.CreateXor(builder.CreateLoad(changeSign), int_8_minus_one),
          changeSign);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi.value)),
                          arg_value);
      builder.CreateBr(not_in_III_quad);
      builder.SetInsertPoint(not_in_III_quad);
    }
    // angle > pi_32&& angle < pi_2(x) sin(x) = -cos(x - pi_32);
    {
      BasicBlock *in_IV_quad = BasicBlock::Create(cont, "in_IV_quad", oldf);
      BasicBlock *not_in_IV_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(builder.CreateLoad(pi_32.value, "pi_32"),
                                      builder.CreateLoad(arg_value),
                                      "arg_greater_pi_32");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value), builder.CreateLoad(pi_2.value, "pi_2"),
          "arg_less_2pi");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_IV_quad, not_in_IV_quad);
      builder.SetInsertPoint(in_IV_quad);
      builder.CreateStore(
          builder.CreateXor(builder.CreateLoad(changeSign), int_8_minus_one),
          changeSign);
      builder.CreateStore(builder.CreateXor(builder.CreateLoad(changedFunction),
                                            int_8_minus_one),
                          changedFunction);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi_32.value)),
                          arg_value);
      builder.CreateBr(not_in_IV_quad);
      builder.SetInsertPoint(not_in_IV_quad);
    }
  } else {

    // angle > pi_half && angle < pi cos(x) = -sin(x - pi_half);
    {
      BasicBlock *in_II_quad = BasicBlock::Create(cont, "in_II_quad", oldf);
      BasicBlock *not_in_II_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(
          builder.CreateLoad(pi_half.value, "pi_half"),
          builder.CreateLoad(arg_value), "arg_greater_pi_half");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value), builder.CreateLoad(pi.value, "pi"),
          "arg_less_pi");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_II_quad, not_in_II_quad);
      builder.SetInsertPoint(in_II_quad);
      builder.CreateStore(
          builder.CreateXor(builder.CreateLoad(changeSign), int_8_minus_one),
          changeSign);
      builder.CreateStore(builder.CreateXor(builder.CreateLoad(changedFunction),
                                            int_8_minus_one),
                          changedFunction);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi_half.value)),
                          arg_value);
      builder.CreateBr(not_in_II_quad);
      builder.SetInsertPoint(not_in_II_quad);
    }
    // angle > pi&& angle < pi_32(x) cos(x) = -cos(x-pi)
    {
      BasicBlock *in_III_quad = BasicBlock::Create(cont, "in_III_quad", oldf);
      BasicBlock *not_in_III_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(builder.CreateLoad(pi.value, "pi"),
                                      builder.CreateLoad(arg_value),
                                      "arg_greater_pi");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value),
          builder.CreateLoad(pi_32.value, "pi_32"), "arg_less_pi_32");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_III_quad, not_in_III_quad);
      builder.SetInsertPoint(in_III_quad);
      builder.CreateStore(
          builder.CreateXor(builder.CreateLoad(changeSign), int_8_minus_one),
          changeSign);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi.value)),
                          arg_value);
      builder.CreateBr(not_in_III_quad);
      builder.SetInsertPoint(not_in_III_quad);
    }
    // angle > pi_32&& angle < pi_2(x) cos(x) = sin(angle - pi_32);
    {
      BasicBlock *in_IV_quad = BasicBlock::Create(cont, "in_IV_quad", oldf);
      BasicBlock *not_in_IV_quad = BasicBlock::Create(cont, "body", oldf);
      generic = builder.CreateICmpSLT(builder.CreateLoad(pi_32.value, "pi_32"),
                                      builder.CreateLoad(arg_value),
                                      "arg_greater_pi_32");
      Value *generic2 = builder.CreateICmpSLT(
          builder.CreateLoad(arg_value), builder.CreateLoad(pi_2.value, "pi_2"),
          "arg_less_2pi");
      generic = builder.CreateAnd(generic, generic2);
      builder.CreateCondBr(generic, in_IV_quad, not_in_IV_quad);
      builder.SetInsertPoint(in_IV_quad);
      builder.CreateStore(builder.CreateXor(builder.CreateLoad(changedFunction),
                                            int_8_minus_one),
                          changedFunction);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                            builder.CreateLoad(pi_32.value)),
                          arg_value);
      builder.CreateBr(not_in_IV_quad);
      builder.SetInsertPoint(not_in_IV_quad);
    }
  }


  // calculate sin and cos
  {
    builder.CreateStore(builder.CreateShl(builder.CreateLoad(arg_value),internal_fxpt.scalarFracBitsAmt() - fxpret.scalarFracBitsAmt()), arg_value);
LLVM_DEBUG(dbgs() << "tre\n");
    auto zero_arg = builder.CreateLoad(zero.value);
    // x=kopp
    builder.CreateStore(builder.CreateLoad(kopp.value), x_value.value);
    // y=0
    builder.CreateStore(zero_arg, y_value.value);

    BasicBlock *epilog_loop = BasicBlock::Create(cont, "epilog_loop", oldf);
    BasicBlock *start_loop = BasicBlock::Create(cont, "start_loop", oldf);

    // i=0
    builder.CreateStore(builder.CreateLoad(zero.value), i_iterator);
    builder.CreateBr(epilog_loop);
    builder.SetInsertPoint(epilog_loop);
    // i < TABLELENGHT
    generic = builder.CreateICmpSLT(
        builder.CreateLoad(i_iterator),
        ConstantInt::get(int_type, TaffoMathConstant::TABLELENGHT));
    // i < size of int
    Value *generic2 = builder.CreateICmpSLT(
        builder.CreateLoad(i_iterator),
        ConstantInt::get(int_type, int_type->getScalarSizeInBits()));
    builder.CreateCondBr(builder.CreateAnd(generic, generic2), start_loop,
                         return_point);
    builder.SetInsertPoint(start_loop);
    LLVM_DEBUG(dbgs() << "4\n");
    // dn = arg >= 0 ? 1 : -1;
    Value *dn = builder.CreateSelect(
        builder.CreateICmpSGE(builder.CreateLoad(arg_value), zero_arg),
        ConstantInt::get(int_type, 1), ConstantInt::get(int_type, -1));

    // xt = x >> i
    Value *xt = builder.CreateAShr(builder.CreateLoad(x_value.value),
                                   builder.CreateLoad(i_iterator));

    // yt = x >> i
    Value *yt = builder.CreateAShr(builder.CreateLoad(y_value.value),
                                   builder.CreateLoad(i_iterator));
LLVM_DEBUG(dbgs() << "5\n");
    // arctan_2power[i]
    generic = builder.CreateGEP(pointer_to_array,
                                {zero_arg, builder.CreateLoad(i_iterator)});

    generic = builder.CreateLoad(generic);

    // dn > 0
    auto dn_greate_zero = builder.CreateICmpSGT(dn, zero_arg);

    // arg = arg + (dn > 0 ? -arctan_2power[i] : arctan_2power[i]);
    generic = builder.CreateSelect(
        dn_greate_zero, builder.CreateSub(zero_arg, generic), generic);

    builder.CreateStore(
        builder.CreateAdd(generic, builder.CreateLoad(arg_value)), arg_value);

    // x = x + (dn > 0 ? -yt : yt);
    generic = builder.CreateSelect(dn_greate_zero,
                                   builder.CreateSub(zero_arg, yt), yt);

    builder.CreateStore(
        builder.CreateAdd(generic, builder.CreateLoad(x_value.value)),
        x_value.value);
    ;
    // y = y + (dn > 0 ? xt : -xt);
    generic = builder.CreateSelect(dn_greate_zero, xt,
                                   builder.CreateSub(zero_arg, xt));
LLVM_DEBUG(dbgs() << "6\n");
    builder.CreateStore(
        builder.CreateAdd(generic, builder.CreateLoad(y_value.value)),
        y_value.value);
    ;
    // i++
    builder.CreateStore(builder.CreateAdd(builder.CreateLoad(i_iterator),
                                          ConstantInt::get(int_type, 1)),
                        i_iterator);
    builder.CreateBr(epilog_loop);
    builder.SetInsertPoint(return_point);
    LLVM_DEBUG(dbgs() << "7\n");
  }
  {
    auto zero_arg = builder.CreateLoad(zero.value);
    auto zero_bool = int_8_zero;
    if (isSin) {
      generic = builder.CreateSelect(
          builder.CreateICmpEQ(builder.CreateLoad(changedFunction), zero_bool),
          builder.CreateLoad(y_value.value), builder.CreateLoad(x_value.value));
      builder.CreateStore(generic, arg_value);
    } else {
      generic = builder.CreateSelect(
          builder.CreateICmpEQ(builder.CreateLoad(changedFunction), zero_bool),
          builder.CreateLoad(x_value.value), builder.CreateLoad(y_value.value));
      builder.CreateStore(generic, arg_value);
    }

    generic = builder.CreateSelect(
        builder.CreateICmpEQ(builder.CreateLoad(changeSign), zero_bool),
        builder.CreateLoad(arg_value),
        builder.CreateSub(zero_arg, builder.CreateLoad(arg_value)));
    builder.CreateStore(generic, arg_value);
  }
  LLVM_DEBUG(dbgs() << "quattro\n");
  if(internal_fxpt.scalarFracBitsAmt() > truefxpret.scalarFracBitsAmt()){
    builder.CreateStore(builder.CreateAShr(builder.CreateLoad(arg_value), internal_fxpt.scalarFracBitsAmt() - truefxpret.scalarFracBitsAmt()), arg_value);
  }
  else if(internal_fxpt.scalarFracBitsAmt() < truefxpret.scalarFracBitsAmt()){
    builder.CreateStore(builder.CreateShl(builder.CreateLoad(arg_value), truefxpret.scalarFracBitsAmt() - internal_fxpt.scalarFracBitsAmt()), arg_value);
    }

  auto ret = builder.CreateLoad(arg_value);
   

  BasicBlock *end = BasicBlock::Create(cont, "end", oldf);
  builder.CreateBr(end);
  builder.SetInsertPoint(end);
  auto tmp10 = builder.Insert(ConstantFP::get(oldf->getReturnType(), 2.0f));
  auto tmp11 = builder.CreateRet(tmp10);
  to_change.push_back({end, {ret}});
  return true;
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
  getFunctionInto(NewFunc, OldFunc, to_change);
  LLVM_DEBUG(dbgs() << "start clone"
                    << "\n");
  
  CloneFunctionInto(NewFunc, OldFunc, VMap, true, Returns);
  LLVM_DEBUG(dbgs() << "end clone"
                    << "\n");
  LLVM_DEBUG(dbgs() << "\nstart fixFun"
                    << "\n");
  FixFunction(NewFunc, OldFunc, VMap, Returns, to_change);
  
  LLVM_DEBUG(dbgs() << "end fixFun"
                    << "\n");
  for (auto &MD : savedMetadata) {
    OldFunc->setMetadata(MD.first, MD.second);
    NewFunc->setMetadata(MD.first, MD.second);
  }
  // LLVM_DEBUG(NewFunc->getParent()->dump());
  LLVM_DEBUG(llvm::verifyFunction(*NewFunc, &dbgs()));
}

// generate first function if never done
// is used to mantain coherent call
void generateFirst(Module *m, llvm::StringRef &fName) {
  Function *tmp = nullptr;
  // get function
  for (auto &i : taffo::HandledFunction::handledFunctions()) {
    if (taffo::start_with(fName, i)) {
      tmp = m->getFunction(i);
    }
    if (tmp == nullptr)
      continue;
    // check if alredy populated
    if (tmp->isDeclaration()) {
      BasicBlock::Create(m->getContext(), fName + ".entry", tmp);
      BasicBlock *where = &(tmp->getEntryBlock());
      IRBuilder<> builder(where, where->getFirstInsertionPt());
      builder.CreateRet(ConstantFP::get(tmp->getReturnType(), 0.0f));
    }
  }
}

bool FloatToFixed::getFunctionInto(
    Function *NewFunc, Function *OldFunc,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  llvm::StringRef fName = OldFunc->getName();

  if (taffo::start_with(fName, "sin") || taffo::start_with(fName, "cos") ||
      taffo::start_with(fName, "_ZSt3cos") ||
      taffo::start_with(fName, "_ZSt3sin")) {
    generateFirst(OldFunc->getParent(), fName);
    return createSinCos(NewFunc, OldFunc, to_change);
  }
  return false;
}

void FloatToFixed::FixFunction(
    Function *NewFunc, Function *OldFunc, ValueToValueMapTy &VMap,
    SmallVectorImpl<ReturnInst *> &Returns,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  llvm::StringRef fName = OldFunc->getName();

  if (taffo::start_with(fName, "sin") || taffo::start_with(fName, "cos") ||
      taffo::start_with(fName, "_ZSt3cos") ||
      taffo::start_with(fName, "_ZSt3sin")) {
    for (auto i : to_change) {
      // to_change.push_back({to_remove, {arg.value, arg_value, body}});
      if (i.first->getName().equals("to_remove")) {
        auto argvalue = VMap[i.second[0]];
        auto arg_value = VMap[i.second[1]];
        auto body = dyn_cast<BasicBlock>((Value *)VMap[i.second[2]]);
        auto old = dyn_cast<BasicBlock>((Value *)VMap[i.first]);

        auto new_arg_store =
            BasicBlock::Create(NewFunc->getContext(), "New_arg_store", NewFunc);
        IRBuilder<> builder(new_arg_store,
                            new_arg_store->getFirstInsertionPt());
        LLVM_DEBUG(argvalue->dump());
        LLVM_DEBUG(arg_value->dump());
        builder.CreateStore(argvalue, arg_value);
        builder.CreateBr(body);
        old->replaceAllUsesWith(new_arg_store);
        old->eraseFromParent();
      }
      // to_change.push_back({end, {ret}});
      if (i.first->getName().equals("end")) {
        auto ret = VMap[i.second[0]];
        auto old = dyn_cast<BasicBlock>((Value *)VMap[i.first]);
        auto true_ret =
            BasicBlock::Create(NewFunc->getContext(), "true_ret", NewFunc);
        IRBuilder<> builder(true_ret, true_ret->getFirstInsertionPt());
        auto generic = builder.CreateRet(ret);
        Returns.clear();
        Returns.push_back(generic);
        old->replaceAllUsesWith(true_ret);
        old->eraseFromParent();
      }
    }
  }
}

} // namespace flttofix