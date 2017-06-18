#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"

#ifndef __LLVM_FLOAT_TO_FIXED_PASS_H__
#define __LLVM_FLOAT_TO_FIXED_PASS_H__


#define N_ANNO_VAR 32
#define NO_FLOAT_ANNO "no_float"

#define DEBUG_TYPE "flttofix"
#define DEBUG_ANNOTATION "annotation"

STATISTIC(FixToFloatCount, "Number of fixed point to floating point conversions");
STATISTIC(FloatToFixCount, "Number of floating point to fixed point conversions");
STATISTIC(FallbackCount, "Number of instructions not replaced by a "
                         "fixed-point-native equivalent");
STATISTIC(ConversionCount, "Number of instructions affected by flttofix");


namespace flttofix {

struct FloatToFixed : public llvm::ModulePass {
  static char ID;
  int fracBitsAmt;
  int bitsAmt;

  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(llvm::Module &M) override;

  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readGlobalAnnotations(llvm::Module &m, bool functionAnnotation = false);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readLocalAnnotations(llvm::Function &f);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readAllLocalAnnotations(llvm::Module &m);
  bool isValidAnnotation(llvm::ConstantExpr *expr);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> removeNoFloatTy(llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> &res);
  void printAnnotatedObj(llvm::Module &m);

  std::vector<llvm::Value*> buildConversionQueueForRootValues(const llvm::ArrayRef<llvm::Value*>& val);
  void performConversion(llvm::Module& m, const std::vector<llvm::Value*>& q);
  llvm::Value *convertSingleValue(llvm::Module& m, llvm::DenseMap<llvm::Value *, llvm::Value *>& operandPool, llvm::Value *val);

  llvm::Value *convertAlloca(llvm::AllocaInst *alloca);
  llvm::Value *convertLoad(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::LoadInst *load);
  llvm::Value *convertStore(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::StoreInst *load);
  llvm::Value *convertPhi(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::PHINode *load);
  llvm::Value *convertSelect(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::SelectInst *sel);
  llvm::Value *convertBinOp(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::Instruction *instr);
  llvm::Value *convertCmp(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::FCmpInst *fcmp);
  llvm::Value *convertCast(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::CastInst *cast);
  llvm::Value *fallback(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::Instruction *unsupp);

  llvm::Value *translateOrMatchOperand(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::Value *val);
  llvm::Value *genConvertFloatToFix(llvm::Value *flt);
  llvm::Constant *convertFloatConstantToFixConstant(llvm::ConstantFP *flt);
  llvm::Value *genConvertFixToFloat(llvm::Value *fix, llvm::Type *destt);

  llvm::Type *getFixedPointType(llvm::LLVMContext &ctxt);
};

}


#endif


