#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
#include "llvm/Support/CommandLine.h"
#include "FixedPointType.h"

#ifndef __LLVM_FLOAT_TO_FIXED_PASS_H__
#define __LLVM_FLOAT_TO_FIXED_PASS_H__


#define DEBUG_TYPE "flttofix"
#define DEBUG_ANNOTATION "annotation"

STATISTIC(FixToFloatCount, "Number of generic fixed point to floating point "
                           "value conversion operations inserted");
STATISTIC(FloatToFixCount, "Number of generic floating point to fixed point "
                           "value conversion operations inserted");
STATISTIC(FallbackCount, "Number of instructions not replaced by a "
                         "fixed-point-native equivalent");
STATISTIC(ConversionCount, "Number of instructions affected by flttofix");
STATISTIC(AnnotationCount, "Number of valid annotations found");


/* flags in conversionPool */
extern llvm::Value *ConversionError;
extern llvm::Value *Unsupported;


namespace flttofix {


bool isFloatType(llvm::Type *srct);


struct ValueInfo {
  bool isBacktrackingNode;
  bool isRoot;
  llvm::SmallPtrSet<llvm::Value*, 5> roots;
  FixedPointType fixpType;  // significant iff origType is a float or a pointer to a float
  llvm::Type *origType;
};


struct FloatToFixed : public llvm::ModulePass {
  static char ID;
  FixedPointType defaultFixpType;
  
  llvm::DenseMap<llvm::Value *, llvm::Value *> operandPool;
  llvm::DenseMap<llvm::Value *, ValueInfo> info;
  
  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(llvm::Module &M) override;

  void readGlobalAnnotations(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *>& res, bool functionAnnotation = false);
  void readLocalAnnotations(llvm::Function &f, llvm::SmallPtrSetImpl<llvm::Value *>& res);
  void readAllLocalAnnotations(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *>& res);
  bool parseAnnotation(llvm::SmallPtrSetImpl<llvm::Value *>& variables, llvm::ConstantExpr *annoPtrInst, llvm::Value *instr);
  void removeNoFloatTy(llvm::SmallPtrSetImpl<llvm::Value *>& res);
  void printAnnotatedObj(llvm::Module &m);

  void buildConversionQueueForRootValues(const llvm::ArrayRef<llvm::Value*>& val, std::vector<llvm::Value*>& res);
  void performConversion(llvm::Module& m, std::vector<llvm::Value*>& q);
  llvm::Value *convertSingleValue(llvm::Module& m, llvm::Value *val, FixedPointType& fixpt);

  llvm::Constant *convertConstant(llvm::Constant *flt, FixedPointType& fixpt);
  llvm::Constant *convertGlobalVariable(llvm::GlobalVariable *glob, FixedPointType& fixpt);
  llvm::Constant *convertConstantExpr(llvm::ConstantExpr *cexp, FixedPointType& fixpt);
  llvm::Constant *convertLiteral(llvm::ConstantFP *flt, llvm::Instruction *, const FixedPointType& fixpt);
  
  llvm::Value *convertInstruction(llvm::Module& m, llvm::Instruction *val);
  llvm::Value *convertAlloca(llvm::AllocaInst *alloca);
  llvm::Value *convertLoad(llvm::LoadInst *load);
  llvm::Value *convertStore(llvm::StoreInst *load);
  llvm::Value *convertGep(llvm::GetElementPtrInst *gep);
  llvm::Value *convertPhi(llvm::PHINode *load);
  llvm::Value *convertSelect(llvm::SelectInst *sel);
  llvm::Value *convertBinOp(llvm::Instruction *instr);
  llvm::Value *convertCmp(llvm::FCmpInst *fcmp);
  llvm::Value *convertCast(llvm::CastInst *cast);
  llvm::Value *fallback(llvm::Instruction *unsupp);

  llvm::Value *matchOp(llvm::Value *val) {
    llvm::Value *res = operandPool[val];
    return res == ConversionError ? nullptr : (res ? res : val);
  };
  llvm::Value *translateOrMatchOperand(llvm::Value *val, FixedPointType& iofixpt, llvm::Instruction *ip = nullptr);
  llvm::Value *translateOrMatchOperandAndType(llvm::Value *val, const FixedPointType& fixpt, llvm::Instruction *ip = nullptr) {
    FixedPointType iofixpt = fixpt;
    llvm::Value *tmp = translateOrMatchOperand(val, iofixpt, ip);
    return genConvertFixedToFixed(tmp, iofixpt, fixpt, ip);
  };
  
  llvm::Value *genConvertFloatToFix(llvm::Value *flt, const FixedPointType& fixpt, llvm::Instruction *ip = nullptr);
  llvm::Value *genConvertFixToFloat(llvm::Value *fix, const FixedPointType& fixpt, llvm::Type *destt);
  llvm::Value *genConvertFixedToFixed(llvm::Value *fix, const FixedPointType& srct, const FixedPointType& destt, llvm::Instruction *ip = nullptr);

  llvm::Type *getLLVMFixedPointTypeForFloatType(llvm::Type *ftype, const FixedPointType& baset);
  llvm::Type *getLLVMFixedPointTypeForFloatValue(llvm::Value *val);
  FixedPointType& fixPType(llvm::Value *val) {
    auto vi = info.find(val);
    assert((vi != info.end()) && "value with no info");
    return vi->getSecond().fixpType;
  };
  
  void cleanup(const std::vector<llvm::Value*>& queue);
};


}


#endif


