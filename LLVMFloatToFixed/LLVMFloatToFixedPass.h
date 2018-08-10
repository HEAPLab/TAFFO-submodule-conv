#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
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
STATISTIC(FunctionCreated, "Number of fixed point function inserted");


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
  int fixpTypeRootDistance = INT_MAX;
  llvm::Type *origType;
};

struct FunInfo {
  llvm::Function* newFun;
  std::vector<std::pair<int,FixedPointType>> fixArgs;
};


struct FloatToFixed : public llvm::ModulePass {
  static char ID;
  FixedPointType defaultFixpType;
  
  llvm::DenseMap<llvm::Value *, llvm::Value *> operandPool;
  llvm::DenseMap<llvm::Function*, std::vector<FunInfo>> functionPool;
  llvm::DenseMap<llvm::Value *, ValueInfo> info;
  
  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(llvm::Module &M) override;

  void readGlobalAnnotations(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *>& res, bool functionAnnotation = false);
  void readLocalAnnotations(llvm::Function &f, llvm::SmallPtrSetImpl<llvm::Value *>& res);
  void readAllLocalAnnotations(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *>& res);
  bool parseAnnotation(llvm::SmallPtrSetImpl<llvm::Value *>& variables, llvm::ConstantExpr *annoPtrInst, llvm::Value *instr);
  void removeNoFloatTy(llvm::SmallPtrSetImpl<llvm::Value *>& res);
  void printAnnotatedObj(llvm::Module &m);

  void optimizeFixedPointTypes(std::vector<llvm::Value*>& queue);
  void buildConversionQueueForRootValues(const llvm::ArrayRef<llvm::Value*>& val, std::vector<llvm::Value*>& res);
  void cleanup(const std::vector<llvm::Value*>& queue);
  void propagateCall(std::vector<llvm::Value *> &vals, llvm::SmallPtrSetImpl<llvm::Value *> &global);
  llvm::Function *createFixFun(llvm::CallInst* call);
  void printConversionQueue(std::vector<llvm::Value*> vals);
  void performConversion(llvm::Module& m, std::vector<llvm::Value*>& q);
  llvm::Value *convertSingleValue(llvm::Module& m, llvm::Value *val, FixedPointType& fixpt);

  llvm::Constant *convertConstant(llvm::Constant *flt, FixedPointType& fixpt);
  llvm::Constant *convertGlobalVariable(llvm::GlobalVariable *glob, FixedPointType& fixpt);
  llvm::Constant *convertConstantExpr(llvm::ConstantExpr *cexp, FixedPointType& fixpt);
  llvm::Constant *convertLiteral(llvm::ConstantFP *flt, llvm::Instruction *, const FixedPointType& fixpt);
  
  llvm::Value *convertInstruction(llvm::Module& m, llvm::Instruction *val, FixedPointType& fixpt);
  llvm::Value *convertAlloca(llvm::AllocaInst *alloca, const FixedPointType& fixpt);
  llvm::Value *convertLoad(llvm::LoadInst *load, FixedPointType& fixpt);
  llvm::Value *convertStore(llvm::StoreInst *load);
  llvm::Value *convertGep(llvm::GetElementPtrInst *gep, FixedPointType& fixpt);
  llvm::Value *convertPhi(llvm::PHINode *load, FixedPointType& fixpt);
  llvm::Value *convertSelect(llvm::SelectInst *sel, FixedPointType& fixpt);
  llvm::Value *convertCall(llvm::CallInst *call, FixedPointType& fixpt);
  llvm::Value *convertRet(llvm::ReturnInst *ret, FixedPointType& fixpt);
  llvm::Value *convertBinOp(llvm::Instruction *instr, const FixedPointType& fixpt);
  llvm::Value *convertCmp(llvm::FCmpInst *fcmp);
  llvm::Value *convertCast(llvm::CastInst *cast, const FixedPointType& fixpt);
  llvm::Value *fallback(llvm::Instruction *unsupp, FixedPointType& fixpt);
  
  std::set<std::string> blackFun = {"printf","log","logf","exp","expf","sqrt","sqrtf","cos","sin",
                                    "tan","acos","asin","atan"};
  bool isSpecialFunction(const llvm::Function* f) { //TODO better logic here
    llvm::StringRef fName = f->getName();
    return fName.startswith("llvm.") || blackFun.count(fName);
  }

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

  llvm::Type *getLLVMFixedPointTypeForFloatType(llvm::Type *ftype, const FixedPointType& baset, bool *hasfloats = nullptr);
  llvm::Type *getLLVMFixedPointTypeForFloatValue(llvm::Value *val);
  FixedPointType& fixPType(llvm::Value *val) {
    auto vi = info.find(val);
    assert((vi != info.end()) && "value with no info");
    return vi->getSecond().fixpType;
  };
  bool hasInfo(llvm::Value *val) {
    return info.find(val) != info.end();
  }
  
};


}


#endif


