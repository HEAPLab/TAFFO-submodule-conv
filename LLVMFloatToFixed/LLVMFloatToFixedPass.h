#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"


#ifndef __LLVM_FLOAT_TO_FIXED_PASS_H__
#define __LLVM_FLOAT_TO_FIXED_PASS_H__


#define N_ANNO_VAR 32
#define NO_FLOAT_ANNO "no_float"


namespace flttofix {

struct FloatToFixed : public llvm::ModulePass {
  static char ID;
  int fracBitsAmt = 16;
  int bitsAmt = 32;

  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(llvm::Module &M) override;

  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readGlobalAnnotations(llvm::Module &m, bool functionAnnotation = false);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readLocalAnnotations(llvm::Function &f);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readAllLocalAnnotations(llvm::Module &m);
  bool isValidAnnotation(llvm::ConstantExpr *expr);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> removeNoFloatTy(llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> res);
  void printAnnotatedObj(llvm::Module &m);

  std::vector<llvm::Value*> buildConversionQueueForRootValues(const llvm::ArrayRef<llvm::Value*>& val);
  void performConversion(llvm::Module& m, const std::vector<llvm::Value*>& q);
  llvm::Value *convertSingleValue(llvm::Module& m, llvm::DenseMap<llvm::Value *, llvm::Value *>& operandPool, llvm::Value *val);

  llvm::Value *convertAlloca(llvm::AllocaInst *alloca);
  llvm::Value *convertLoad(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::LoadInst *load);
  llvm::Value *convertStore(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::StoreInst *load);
  llvm::Value *fallback(llvm::Module &m, llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::Instruction *unsupp);
  
  llvm::Value *translateOrMatchOperand(llvm::DenseMap<llvm::Value *, llvm::Value *>& op, llvm::Value *val);
  llvm::Value *genConvertFloatToFix(llvm::Value *flt);
  llvm::Constant *convertFloatConstantToFixConstant(llvm::ConstantFP *flt);
  llvm::Value *genConvertFixToFloat(llvm::Value *fix, llvm::Type *destt);
};

}


#endif


