#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"


#ifndef __LLVM_FLOAT_TO_FIXED_PASS_H__
#define __LLVM_FLOAT_TO_FIXED_PASS_H__


#define N_ANNO_VAR 32
#define NO_FLOAT_ANNO "no_float"


namespace flttofix {

struct FloatToFixed : public llvm::ModulePass {
  static char ID;

  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(llvm::Module &M) override;
  
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readGlobalAnnotations(llvm::Module &m, bool functionAnnotation = false);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readLocalAnnotations(llvm::Function &f);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> readAllLocalAnnotations(llvm::Module &m);
  bool isValidAnnotation(llvm::ConstantExpr *expr);
  llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> removeNoFloatTy(llvm::SmallPtrSet<llvm::Value*, N_ANNO_VAR> res);
  void printAnnotatedObj(llvm::Module &m);
  
  std::vector<llvm::Value*> buildConversionQueueForRootValue(llvm::Value *val);
};

}


#endif


