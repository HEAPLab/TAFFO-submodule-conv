#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CallSite.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/CommandLine.h"
#include "TypeUtils.h"
#include "Metadata.h"
#include "FixedPointType.h"
#include "InputInfo.h"

#ifndef __LLVM_FLOAT_TO_FIXED_PASS_H__
#define __LLVM_FLOAT_TO_FIXED_PASS_H__


#define DEBUG_TYPE "taffo-conversion"
#define DEBUG_ANNOTATION "annotation"


STATISTIC(FixToFloatCount, "Number of generic fixed point to floating point value conversion operations inserted");
STATISTIC(FloatToFixCount, "Number of generic floating point to fixed point value conversion operations inserted");
STATISTIC(FixToFloatWeight, "Number of generic fixed point to floating point value conversion operations inserted,"
  " weighted by the loop depth");
STATISTIC(FloatToFixWeight, "Number of generic floating point to fixed point value conversion operations inserted,"
  " weighted by the loop depth");
STATISTIC(FallbackCount, "Number of instructions not replaced by a fixed-point-native equivalent");
STATISTIC(ConversionCount, "Number of instructions affected by flttofix");
STATISTIC(MetadataCount, "Number of valid Metadata found");
STATISTIC(FunctionCreated, "Number of fixed point function inserted");


/* flags in conversionPool */
extern llvm::Value *ConversionError;
extern llvm::Value *Unsupported;


namespace flttofix {


struct ValueInfo {
  bool isBacktrackingNode;
  bool isRoot;
  llvm::SmallPtrSet<llvm::Value*, 5> roots;
  unsigned int fixpTypeRootDistance = UINT_MAX;
  
  /* Disable type conversion even if the instruction
   * produces a floating point value */
  bool noTypeConversion = false;
  
  // significant iff origType is a float or a pointer to a float
  // and if operation == Convert
  FixedPointType fixpType;
  llvm::Type *origType = nullptr;
};


struct PHIInfo {
  llvm::PHINode *phi;
  llvm::Value *placeh_noconv;
  llvm::Value *placeh_conv;
};


struct FloatToFixed : public llvm::ModulePass {
  static char ID;
  FixedPointType defaultFixpType;
  
  /** Map from original values to converted values.
   *  Values not to be converted do not appear in the map.
   *  Values which have not been converted successfully are mapped to
   *  one of two sentinel values, ConversionError or Unsupported. */
  llvm::DenseMap<llvm::Value *, llvm::Value *> operandPool;
  
  /** Map from original function (as cloned by Initializer)
   *  to function cloned by this pass in order to change argument
   *  and return values */
  llvm::DenseMap<llvm::Function*, llvm::Function*> functionPool;
  
  /* to not be accessed directly, use valueInfo() */
  llvm::DenseMap<llvm::Value *, std::shared_ptr<ValueInfo>> info;
  
  llvm::SmallVector<PHIInfo, 8> phiReplacementData;
  
  FloatToFixed(): ModulePass(ID) { };
  void getAnalysisUsage(llvm::AnalysisUsage &) const override;
  bool runOnModule(llvm::Module &M) override;

  void readGlobalMetadata(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *> &res, bool functionAnnotation = false);
  void readLocalMetadata(llvm::Function &f, llvm::SmallPtrSetImpl<llvm::Value *> &res);
  void readAllLocalMetadata(llvm::Module &m, llvm::SmallPtrSetImpl<llvm::Value *> &res);
  bool parseMetaData(llvm::SmallPtrSetImpl<llvm::Value *> *variables, mdutils::MDInfo *fpInfo, llvm::Value *instr);
  void removeNoFloatTy(llvm::SmallPtrSetImpl<llvm::Value *>& res);
  void printAnnotatedObj(llvm::Module &m);
  
  void openPhiLoop(llvm::PHINode *phi);
  void closePhiLoops();
  void sortQueue(std::vector<llvm::Value*> &vals);
  void cleanup(const std::vector<llvm::Value*>& queue);
  void propagateCall(std::vector<llvm::Value *> &vals, llvm::SmallPtrSetImpl<llvm::Value *> &global);
  llvm::Function *createFixFun(llvm::CallSite* call);
  void printConversionQueue(std::vector<llvm::Value*> vals);
  void performConversion(llvm::Module& m, std::vector<llvm::Value*>& q);
  llvm::Value *convertSingleValue(llvm::Module& m, llvm::Value *val, FixedPointType& fixpt);
  
  llvm::Value *createPlaceholder(llvm::Type *type, llvm::BasicBlock *where, llvm::StringRef name);

  /* convert* functions return nullptr if the conversion cannot be
   * recovered, and Unsupported to trigger the fallback behavior */
  
  llvm::Constant *convertConstant(llvm::Constant *flt, FixedPointType& fixpt);
  llvm::Constant *convertGlobalVariable(llvm::GlobalVariable *glob, FixedPointType& fixpt);
  llvm::Constant *convertConstantExpr(llvm::ConstantExpr *cexp, FixedPointType& fixpt);
  llvm::Constant *convertConstantAggregate(llvm::ConstantAggregate *cag, FixedPointType& fixpt);
  llvm::Constant *convertConstantDataSequential(llvm::ConstantDataSequential *, const FixedPointType&);
  template <class T> llvm::Constant *createConstantDataSequential(llvm::ConstantDataSequential *, const FixedPointType&);
  llvm::Constant *convertLiteral(llvm::ConstantFP *flt, llvm::Instruction *, const FixedPointType&);
  bool convertAPFloat(llvm::APFloat, llvm::APSInt&, llvm::Instruction *, const FixedPointType&);
  
  llvm::Value *convertInstruction(llvm::Module& m, llvm::Instruction *val, FixedPointType& fixpt);
  llvm::Value *convertAlloca(llvm::AllocaInst *alloca, const FixedPointType& fixpt);
  llvm::Value *convertLoad(llvm::LoadInst *load, FixedPointType& fixpt);
  llvm::Value *convertStore(llvm::StoreInst *load);
  llvm::Value *convertGep(llvm::GetElementPtrInst *gep, FixedPointType& fixpt);
  llvm::Value *convertExtractValue(llvm::ExtractValueInst *exv, FixedPointType& fixpt);
  llvm::Value *convertInsertValue(llvm::InsertValueInst *inv, FixedPointType& fixpt);
  llvm::Value *convertPhi(llvm::PHINode *load, FixedPointType& fixpt);
  llvm::Value *convertSelect(llvm::SelectInst *sel, FixedPointType& fixpt);
  llvm::Value *convertCall(llvm::CallSite *call, FixedPointType& fixpt);
  llvm::Value *convertRet(llvm::ReturnInst *ret, FixedPointType& fixpt);
  llvm::Value *convertBinOp(llvm::Instruction *instr, const FixedPointType& fixpt);
  llvm::Value *convertCmp(llvm::FCmpInst *fcmp);
  llvm::Value *convertCast(llvm::CastInst *cast, const FixedPointType& fixpt);
  llvm::Value *fallback(llvm::Instruction *unsupp, FixedPointType& fixpt);
  
  /** Returns if a function is a library function which shall not
   *  be cloned.
   *  @param f The function to check */
  bool isSpecialFunction(const llvm::Function* f) {
    llvm::StringRef fName = f->getName();
    return fName.startswith("llvm.") || f->getBasicBlockList().size() == 0;
  };

  /** Returns the converted Value matching a non-converted Value.
   *  @param val The non-converted value to match.
   *  @returns nullptr if the value has not been converted properly,
   *    the converted value if the original value was converted,
   *    or the original value itself if it does not require conversion. */
  llvm::Value *matchOp(llvm::Value *val) {
    llvm::Value *res = operandPool[val];
    return res == ConversionError ? nullptr : (res ? res : val);
  };
  
  /** Returns a fixed point Value from any Value, whether it should be
   *  converted or not.
   *  @param val The non-converted value. Must be of a primitive floating-point
   *    non-reference LLVM type (in other words, ints, pointers, arrays, struct are
   *    not allowed); use matchOp() for values of those types.
   *  @param iofixpt A reference to a fixed point type. On input,
   *    it must contain the preferred fixed point type required
   *    for the returned Value. On output, it will contain the
   *    actual fixed point type of the returned Value (which may or
   *    may not be different than the input type).
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns A fixed point value corresponding to val or nullptr if
   *    val was to be converted but its conversion failed. */
  llvm::Value *translateOrMatchOperand(llvm::Value *val, FixedPointType& iofixpt, llvm::Instruction *ip = nullptr);
  /** Returns a fixed point Value from any Value, whether it should be
   *  converted or not, if possible.
   *  @param val The non-converted value.
   *  @param iofixpt A reference to a fixed point type. On input,
   *    it must contain the preferred fixed point type required
   *    for the returned Value. On output, it will contain the
   *    actual fixed point type of the returned Value (which may or
   *    may not be different than the input type).
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns A fixed point value corresponding to val or nullptr if
   *    val was to be converted but its conversion failed. */
  llvm::Value *translateOrMatchAnyOperand(llvm::Value *val, FixedPointType& iofixpt, llvm::Instruction *ip = nullptr) {
    llvm::Value *res;
    if (val->getType()->getNumContainedTypes() > 0) {
      if (llvm::Constant *cst = llvm::dyn_cast<llvm::Constant>(val)) {
        res = convertConstant(cst, iofixpt);
      } else {
        res = matchOp(val);
        if (res)
          iofixpt = valueInfo(res)->fixpType;
      }
    } else {
      res = translateOrMatchOperand(val, iofixpt, ip);
    }
    return res;
  }
  /** Returns a fixed point Value of a specified fixed point type from any
   *  Value, whether it should be converted or not.
   *  @param val The non-converted value. Must be of a primitive floating-point
   *    non-reference LLVM type (in other words, ints, pointers, arrays, struct are
   *    not allowed); use matchOp() for values of those types.
   *  @param fixpt The fixed point type of the value returned.
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns A fixed point value corresponding to val of type fixpt
   *    or nullptr if val was to be converted but its conversion failed.  */
  llvm::Value *translateOrMatchOperandAndType(llvm::Value *val, const FixedPointType& fixpt, llvm::Instruction *ip = nullptr) {
    FixedPointType iofixpt = fixpt;
    llvm::Value *tmp = translateOrMatchOperand(val, iofixpt, ip);
    return genConvertFixedToFixed(tmp, iofixpt, fixpt, ip);
  };
  /** Returns a fixed point Value of a specified fixed point type from any
   *  Value, whether it should be converted or not, if possible.
   *  @param val The non-converted value.
   *  @param fixpt The fixed point type of the value returned.
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns A fixed point value corresponding to val of type fixpt
   *    or nullptr if val was to be converted but its conversion failed.
   *    An assertion is raised if the value cannot be converted to
   *    the specified type (for example if it is a pointer)  */
  llvm::Value *translateOrMatchAnyOperandAndType(llvm::Value *val, const FixedPointType& fixpt, llvm::Instruction *ip = nullptr) {
    llvm::Value *res;
    if (val->getType()->getNumContainedTypes() > 0) {
      if (llvm::Constant *cst = llvm::dyn_cast<llvm::Constant>(val)) {
        FixedPointType tmpfixpt = fixpt;
        res = convertConstant(cst, tmpfixpt);
        assert(fixpt == tmpfixpt && "type mismatch on reference Value");
      } else {
        res = matchOp(val);
        assert(fixpt == valueInfo(res)->fixpType && "type mismatch on reference Value");
      }
    } else {
      res = translateOrMatchOperandAndType(val, fixpt, ip);
    }
    return res;
  };
  
  llvm::Value *fallbackMatchValue(llvm::Value *fallval, llvm::Type *origType, llvm::Instruction *ip = nullptr) {
    llvm::Value *cvtfallval = operandPool[fallval];
    
    if (cvtfallval == ConversionError) {
      LLVM_DEBUG(llvm::dbgs() << "error: bail out reverse match of " << *fallval << "\n");
      return nullptr;
    }
    if (!hasInfo(cvtfallval))
      return cvtfallval;
    if (valueInfo(cvtfallval)->noTypeConversion)
      return cvtfallval;
    
    if (!ip) {
      ip = llvm::dyn_cast<llvm::Instruction>(cvtfallval);
      if (ip)
        ip = ip->getNextNode();
      assert(ip && "ip mandatory for non-instruction values");
    }
    
    /*Nel caso in cui la chiave (valore rimosso in precedenze) è un float
      il rispettivo value è un fix che deve essere convertito in float per retrocompatibilità.
      Se la chiave non è un float allora uso il rispettivo value associato così com'è.*/
    if (cvtfallval->getType()->isPointerTy()) {
      llvm::BitCastInst *bc = new llvm::BitCastInst(cvtfallval, origType);
      cpMetaData(bc,cvtfallval);
      bc->insertBefore(ip);
      return bc;
    }
    if (origType->isFloatingPointTy())
      return genConvertFixToFloat(cvtfallval, fixPType(cvtfallval), origType);
    return cvtfallval;
  }
  
  /** Generate code for converting the value of a scalar from floating point to
   *  fixed point.
   *  @param flt A floating point scalar value.
   *  @param fixpt The fixed point type of the output
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns The converted value. */
  llvm::Value *genConvertFloatToFix(llvm::Value *flt, const FixedPointType& fixpt, llvm::Instruction *ip = nullptr);
  /** Generate code for converting the value of a scalar from fixed point to
   *  floating point.
   *  @param flt A fixed point scalar value.
   *  @param fixpt The fixed point type of the input
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns The converted value. */
  llvm::Value *genConvertFixToFloat(llvm::Value *fix, const FixedPointType& fixpt, llvm::Type *destt);
  /** Generate code for converting between two fixed point formats.
   *  @param flt A fixed point scalar value.
   *  @param scrt The fixed point type of the input
   *  @param destt The fixed point type of the output
   *  @param ip The instruction which will use the returned value.
   *    Used for placing generated fixed point runtime conversion code in
   *    case val was not to be converted statically. Not required if val
   *    is an instruction or a constant.
   *  @returns The converted value. */
  llvm::Value *genConvertFixedToFixed(llvm::Value *fix, const FixedPointType& srct, const FixedPointType& destt, llvm::Instruction *ip = nullptr);

  /** Transforms a pre-existing LLVM type to a new LLVM
   *  type with integers instead of floating point depending on a
   *  fixed point type specification.
   *  @param fptype The original type
   *  @param baset The fixed point type
   *  @param hasfloats If non-null, points to a bool which, on return,
   *    will be true if at least one floating point type to transform to
   *    fixed point was encountered.
   *  @returns The new LLVM type.  */
  llvm::Type *getLLVMFixedPointTypeForFloatType(llvm::Type *ftype, const FixedPointType& baset, bool *hasfloats = nullptr);
  
  llvm::Instruction *getFirstInsertionPointAfter(llvm::Instruction *i) {
    llvm::Instruction *ip = i->getNextNode();
    if (!ip) {
      LLVM_DEBUG(llvm::dbgs() << "warning: getFirstInsertionPointAfter on a BB-terminating inst\n");
      return nullptr;
    }
    if (llvm::isa<llvm::PHINode>(ip))
      ip = ip->getParent()->getFirstNonPHI();
    return ip;
  }
  
  llvm::Type *getLLVMFixedPointTypeForFloatValue(llvm::Value *val);
  
  std::shared_ptr<ValueInfo> newValueInfo(llvm::Value *val) {
    LLVM_DEBUG(llvm::dbgs() << "new valueinfo for " << *val << "\n");
    auto vi = info.find(val);
    if (vi == info.end()) {
      info[val] = std::make_shared<ValueInfo>(ValueInfo());
      return info[val];
    } else {
      assert(false && "value already has info!");
    }
  }
  std::shared_ptr<ValueInfo> demandValueInfo(llvm::Value *val, bool *isNew = nullptr) {
    LLVM_DEBUG(llvm::dbgs() << "new valueinfo for " << *val << "\n");
    auto vi = info.find(val);
    if (vi == info.end()) {
      if (isNew) *isNew = true;
      info[val] = std::make_shared<ValueInfo>(ValueInfo());
      return info[val];
    } else {
      if (isNew) *isNew = false;
      return vi->getSecond();
    }
  }
  std::shared_ptr<ValueInfo> valueInfo(llvm::Value *val) {
    auto vi = info.find(val);
    assert((vi != info.end()) && "value with no info");
    return vi->getSecond();
  };
  FixedPointType& fixPType(llvm::Value *val) {
    auto vi = info.find(val);
    assert((vi != info.end()) && "value with no info");
    return vi->getSecond()->fixpType;
  };
  bool hasInfo(llvm::Value *val) {
    return info.find(val) != info.end();
  };
  
  bool isConvertedFixedPoint(llvm::Value *val) {
    if (!hasInfo(val))
      return false;
    std::shared_ptr<ValueInfo> vi = valueInfo(val);
    if (vi->noTypeConversion)
      return false;
    if (vi->fixpType.isInvalid())
      return false;
    llvm::Type *fuwt = taffo::fullyUnwrapPointerOrArrayType(vi->origType);
    if (!fuwt->isStructTy()) {
      if (!taffo::isFloatType(vi->origType))
        return false;
    }
    if (val->getType() == vi->origType)
      return false;
    return true;
  }
  bool isFloatingPointToConvert(llvm::Value *val) {
    if (llvm::isa<llvm::Argument>(val))
      // function arguments to be converted are substituted by placeholder
      // values in the function cloning stage.
      // Besides, they cannot be replaced without recreating the
      // function, thus they never fit the requirements for being
      // converted.
      return false;
    if (!hasInfo(val))
      return false;
    std::shared_ptr<ValueInfo> vi = valueInfo(val);
    if (vi->noTypeConversion)
      return false;
    if (vi->fixpType.isInvalid())
      return false;
    llvm::Type *fuwt = taffo::fullyUnwrapPointerOrArrayType(val->getType());
    if (!fuwt->isStructTy()) {
      if (!taffo::isFloatType(val->getType()))
        return false;
    }
    return true;
  }
  llvm::Value *cpMetaData(llvm::Value* dst, llvm::Value* src, llvm::Instruction* target = nullptr) {
    using namespace llvm;
    MDNode *md = nullptr;
    MDNode *targetMD = nullptr;
    if (Instruction *from = dyn_cast<Instruction>(src)) {
      md = from->getMetadata(INPUT_INFO_METADATA);
      targetMD = from->getMetadata(TARGET_METADATA);
    } else if (GlobalObject *from = dyn_cast<GlobalObject>(src)) {
      md = from->getMetadata(INPUT_INFO_METADATA);
      targetMD = from->getMetadata(TARGET_METADATA);
    }
    if (!md && target) {
      md = target->getMetadata(INPUT_INFO_METADATA);
      targetMD = target->getMetadata(TARGET_METADATA);
    }

    if (md) {
      if (Instruction *to = dyn_cast<Instruction>(dst))
        to->setMetadata(INPUT_INFO_METADATA, md);
      else if (GlobalObject *to = dyn_cast<GlobalObject>(dst))
        to->setMetadata(INPUT_INFO_METADATA, md);
    }

    if (targetMD) {
      if (Instruction *to = dyn_cast<Instruction>(dst))
        to->setMetadata(TARGET_METADATA, targetMD);
      else if (GlobalObject *to = dyn_cast<GlobalObject>(dst))
        to->setMetadata(TARGET_METADATA, targetMD);
    }

    return dst;
  }
  void updateFPTypeMetadata(llvm::Value *v, bool isSigned, int fracBitsAmt, int bitsAmt) {
    using namespace llvm;
    using namespace mdutils;

    MetadataManager& mdmgr = MetadataManager::getMetadataManager();
    InputInfo* ii = dyn_cast_or_null<InputInfo>(mdmgr.retrieveMDInfo(v));
    if (!ii)
      return;

    InputInfo* newII = cast<InputInfo>(ii->clone());
    newII->IType.reset(new FPType(bitsAmt, fracBitsAmt, isSigned));

    mdmgr.setMDInfoMetadata(v, newII);
  }

  int getLoopNestingLevelOfValue(llvm::Value *v);
};


}


#endif


