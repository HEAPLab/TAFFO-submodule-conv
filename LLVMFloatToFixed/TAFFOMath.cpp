#include "TAFFOMath.h"
#include "llvm/IR/Verifier.h"
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
                            FixedPointType &fxpret) {
  for (auto i : oldf->users()) {
    if (isa<llvm::CallInst>(i)) {
      llvm::CallInst *callInst = dyn_cast<llvm::CallInst>(i);
      if (ref->hasInfo(callInst)) {
        fxpret = ref->valueInfo(callInst)->fixpType;
        LLVM_DEBUG(dbgs() << "\nReturn type: Scalar bits "
                          << fxpret.scalarBitsAmt() << "Fractional bits"
                          << fxpret.scalarFracBitsAmt() << "\n");
      }
    }
  }
}

inline void getFixedFromArg(FloatToFixed *ref, Function *oldf,
                            FixedPointType &fxparg, int n) {

  for (auto arg = oldf->arg_begin(); arg != oldf->arg_end(); ++arg) {
    if (n == 0) {
      if (ref->hasInfo(arg)) {
        fxparg = ref->valueInfo(arg)->fixpType;
        LLVM_DEBUG(dbgs() << "\nReturn arg: Scalar bits "
                          << fxparg.scalarBitsAmt() << "Fractional bits"
                          << fxparg.scalarFracBitsAmt() << "\n");
      }
      return;
    }
    --n;
    assert(n >= 0 && "Try to access invalid argument");
  }
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
void FloatToFixed::createSinCos(
    llvm::Function *newfs, llvm::Function *oldf,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  //
  Value *generic;
  bool isSin = oldf->getName().find("sin") == 0;
  // retrive context used in later instruction
  llvm::LLVMContext &cont(oldf->getContext());
  DataLayout dataLayout(oldf->getParent());
  LLVM_DEBUG(dbgs() << "\nGet Context " << &cont << "\n");
  // get first basick block of function
  BasicBlock::Create(cont, "Entry.sin", oldf);
  BasicBlock *where = &(oldf->getEntryBlock());
  LLVM_DEBUG(dbgs() << "\nGet entry point " << where);
  IRBuilder<> builder(where, where->getFirstInsertionPt());
  // get return type fixed point
  flttofix::FixedPointType fxpret;
  flttofix::FixedPointType fxparg;
  getFixedFromRet(this, oldf, fxpret);
  // get argument fixed point
  getFixedFromArg(this, oldf, fxparg, 0);
  TaffoMathConstant::pair_ftp_value<llvm::Value *> arg(fxparg);
  arg.value = oldf->arg_begin();
  auto int_type = fxparg.scalarToLLVMType(cont);

  assert(fxparg == fxpret);
  // create local variable
  TaffoMathConstant::pair_ftp_value<llvm::Value *> x_value(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Value *> y_value(fxpret);
  x_value.value = builder.CreateAlloca(int_type, nullptr, "x");
  y_value.value = builder.CreateAlloca(int_type, nullptr, "y");
  auto changeSign =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "changeSign");
  auto changedFunction =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "changefunc");
  auto specialAngle =
      builder.CreateAlloca(Type::getInt8Ty(cont), nullptr, "specialAngle");
  Value *arg_value = builder.CreateAlloca(int_type, nullptr, "arg");
  Value *i_iterator = builder.CreateAlloca(int_type, nullptr, "arg");

  // create pi variable
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_2(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_32(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> pi_half(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> kopp(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> zero(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> one(fxpret);
  TaffoMathConstant::pair_ftp_value<llvm::Constant *> minus_one(fxpret);
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
      cont, this, TaffoMathConstant::Kopp, fxpret, kopp.value, kopp.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::zero, fxpret, zero.value, zero.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::one, fxpret, one.value, one.fpt);
  TaffoMathConstant::createFixedPointFromConst(
      cont, this, TaffoMathConstant::minus_one, fxpret, minus_one.value,
      minus_one.fpt);
  pi.value = createGlobalConst(
      oldf->getParent(), "pi", pi.fpt.scalarToLLVMType(cont), pi.value,
      dataLayout.getPrefTypeAlignment(pi.fpt.scalarToLLVMType(cont)));
  pi_2.value = createGlobalConst(
      oldf->getParent(), "pi_2", pi_2.fpt.scalarToLLVMType(cont), pi_2.value,
      dataLayout.getPrefTypeAlignment(pi_2.fpt.scalarToLLVMType(cont)));
  pi_32.value = createGlobalConst(
      oldf->getParent(), "pi_32", pi_32.fpt.scalarToLLVMType(cont), pi_32.value,
      dataLayout.getPrefTypeAlignment(pi_32.fpt.scalarToLLVMType(cont)));
  pi_half.value = createGlobalConst(
      oldf->getParent(), "pi_half", pi_half.fpt.scalarToLLVMType(cont),
      pi_half.value,
      dataLayout.getPrefTypeAlignment(pi_half.fpt.scalarToLLVMType(cont)));
  kopp.value = createGlobalConst(
      oldf->getParent(), "kopp", kopp.fpt.scalarToLLVMType(cont), kopp.value,
      dataLayout.getPrefTypeAlignment(kopp.fpt.scalarToLLVMType(cont)));
  zero.value = createGlobalConst(
      oldf->getParent(), "zero", zero.fpt.scalarToLLVMType(cont), zero.value,
      dataLayout.getPrefTypeAlignment(zero.fpt.scalarToLLVMType(cont)));
  one.value = createGlobalConst(
      oldf->getParent(), "one", one.fpt.scalarToLLVMType(cont), one.value,
      dataLayout.getPrefTypeAlignment(one.fpt.scalarToLLVMType(cont)));
  minus_one.value = createGlobalConst(
      oldf->getParent(), "minus_one", minus_one.fpt.scalarToLLVMType(cont),
      minus_one.value,
      dataLayout.getPrefTypeAlignment(minus_one.fpt.scalarToLLVMType(cont)));
  /** create arctan table
   **/
  LLVM_DEBUG(dbgs() << "Create arctan table"
                    << "\n");
  TaffoMathConstant::pair_ftp_value<llvm::Constant *,
                                    TaffoMathConstant::TABLELENGHT>
      arctan_2power;
  for (int i = 0; i < TaffoMathConstant::TABLELENGHT; i++) {
    auto &fixpt = arctan_2power.fpt[i];
    auto &value = arctan_2power.value[i];
    TaffoMathConstant::createFixedPointFromConst(
        cont, this, TaffoMathConstant::arctan_2power[i], fxpret, value, fixpt);
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
      createGlobalConst(oldf->getParent(), "arctan_g", arctanArrayType,
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

  // handle negative
  if (isSin) {
    // sin(-x) == -sin(x)
    {
      BasicBlock *true_greater_zero =
          BasicBlock::Create(cont, "true_greater_zero", oldf);
      BasicBlock *false_greater_zero = BasicBlock::Create(cont, "body", oldf);

      generic = builder.CreateICmpSLT(builder.CreateLoad(arg_value, "arg"),
                                      builder.CreateLoad(zero.value));
      generic =
          builder.CreateCondBr(generic, true_greater_zero, false_greater_zero);
      builder.SetInsertPoint(true_greater_zero);
      generic = builder.CreateSub(builder.CreateLoad(zero.value),
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
                                      builder.CreateLoad(zero.value));
      generic =
          builder.CreateCondBr(generic, true_greater_zero, false_greater_zero);
      builder.SetInsertPoint(true_greater_zero);
      generic = builder.CreateSub(builder.CreateLoad(zero.value),
                                  builder.CreateLoad(arg_value));
      builder.CreateStore(generic, arg_value);
      builder.CreateBr(false_greater_zero);
      builder.SetInsertPoint(false_greater_zero);
    }
  }

  // fix arg > 2pi
  {
    BasicBlock *bigger_than_2pi =
        BasicBlock::Create(cont, "bigger_than_2pi", oldf);
    BasicBlock *cmp_bigger_than_2pi =
        BasicBlock::Create(cont, "cmp_bigger_than_2pi", oldf);
    BasicBlock *end_loop = BasicBlock::Create(cont, "body", oldf);
    builder.CreateBr(cmp_bigger_than_2pi);
    builder.SetInsertPoint(cmp_bigger_than_2pi);
    generic = builder.CreateICmpSLE(builder.CreateLoad(pi_2.value),
                                    builder.CreateLoad(arg_value));
    builder.CreateCondBr(generic, bigger_than_2pi, end_loop);
    builder.SetInsertPoint(bigger_than_2pi);
    builder.CreateStore(builder.CreateSub(builder.CreateLoad(arg_value),
                                          builder.CreateLoad(pi_2.value)),
                        arg_value);
    builder.CreateBr(cmp_bigger_than_2pi);
    builder.SetInsertPoint(end_loop);
  }

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
      builder.CreateStore(builder.CreateLoad(one.value), y_value.value);
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
      builder.CreateStore(builder.CreateLoad(minus_one.value), x_value.value);
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
      builder.CreateStore(builder.CreateLoad(minus_one.value), y_value.value);
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
      builder.CreateStore(builder.CreateLoad(one.value), x_value.value);
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
  auto ret = builder.CreateLoad(arg_value);

  BasicBlock *end = BasicBlock::Create(cont, "end", oldf);
  builder.CreateBr(end);
  builder.SetInsertPoint(end);
  auto tmp10 = builder.Insert(ConstantFP::get(oldf->getReturnType(), 2.0f));
  auto tmp11 = builder.CreateRet(tmp10);
  to_change.push_back({end, {ret}});
}

void FloatToFixed::populateFunction(
    Function *NewFunc, Function *OldFunc, ValueToValueMapTy &VMap,
    bool ModuleLevelChanges, SmallVectorImpl<ReturnInst *> &Returns,
    const char *NameSuffix, ClonedCodeInfo *CodeInfo,
    ValueMapTypeRemapper *TypeMapper, ValueMaterializer *Materializer) {
  llvm::SmallVector<std::pair<unsigned int, llvm::MDNode *>, 10> savedMetadata;
  OldFunc->getAllMetadata(savedMetadata);
  // remove old body
  if (OldFunc->begin() == OldFunc->end()) {
    LLVM_DEBUG(dbgs() << "\ndelete body"
                      << "\n");
    OldFunc->deleteBody();
  }

  SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3> to_change;
  getFunctionInto(NewFunc, OldFunc, to_change);
  LLVM_DEBUG(dbgs() << "\nstart clone"
                    << "\n");

  LLVM_DEBUG(dbgs() << "\nNew func " << NewFunc << "\n");
  CloneFunctionInto(NewFunc, OldFunc, VMap, true, Returns);
  LLVM_DEBUG(dbgs() << "end clone"
                    << "\n");
  {

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

void FloatToFixed::getFunctionInto(
    Function *NewFunc, Function *OldFunc,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change) {

  llvm::StringRef fName = OldFunc->getName();
  LLVM_DEBUG(dbgs() << "inizio\n");
  if (taffo::start_with(fName, "sin") || taffo::start_with(fName, "cos")) {
    generateFirst(OldFunc->getParent(), fName);
    createSinCos(NewFunc, OldFunc, to_change);
  }
}
} // namespace flttofix