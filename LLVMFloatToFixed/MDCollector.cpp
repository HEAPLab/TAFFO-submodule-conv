#include <sstream>
#include <iostream>
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"
#include "Metadata.h"


using namespace llvm;
using namespace flttofix;
using namespace mdutils;
using namespace taffo;


void FloatToFixed::readGlobalMetadata(Module &m, SmallPtrSetImpl<Value *> &variables, bool functionAnnotation)
{
  MetadataManager &MDManager = MetadataManager::getMetadataManager();
  
  for (GlobalVariable &gv : m.globals()) {
    InputInfo *II = MDManager.retrieveInputInfo(gv);
    StructInfo *SI;
    if (II != nullptr && II->IType != nullptr) {
      if (!II->IEnableConversion)
        continue;
      if (FPType *fpInfo = dyn_cast<FPType>(II->IType.get())) {
        parseMetaData(variables, fpInfo, &gv);
      }
    } else if ((SI = MDManager.retrieveStructInfo(gv))) {
      parseStructMetaData(variables, SI, &gv);
    }
  }
  if (functionAnnotation)
    removeNoFloatTy(variables);
}


void FloatToFixed::readLocalMetadata(Function &f, SmallPtrSetImpl<Value *> &variables)
{
  MetadataManager &MDManager = MetadataManager::getMetadataManager();

  SmallVector<mdutils::MDInfo*, 5> argsII;
  MDManager.retrieveArgumentInputInfo(f, argsII);
  auto arg = f.arg_begin();
  for (auto itII = argsII.begin(); itII != argsII.end(); itII++) {
    if (*itII != nullptr) {
      if (InputInfo *ii = dyn_cast<InputInfo>(*itII)) {
        if (ii->IEnableConversion && ii->IType) {
          FPType *fpInfo = dyn_cast<FPType>(ii->IType.get());
          if (fpInfo)
            parseMetaData(variables, fpInfo, arg);
        }
      } else if (StructInfo *si = dyn_cast<StructInfo>(*itII)) {
        parseStructMetaData(variables, si, arg);
      }
    }
    arg++;
  }

  for (inst_iterator iIt = inst_begin(&f), iItEnd = inst_end(&f); iIt != iItEnd; iIt++) {
    InputInfo *II = MDManager.retrieveInputInfo(*iIt);
    StructInfo *SI;
    if (II != nullptr && II->IType != nullptr) {
      if (!II->IEnableConversion)
        continue;
      if (FPType *fpInfo = dyn_cast<FPType>(II->IType.get())) {
        parseMetaData(variables, fpInfo, &(*iIt));
      }
    } else if ((SI = MDManager.retrieveStructInfo(*iIt))) {
      parseStructMetaData(variables, SI, &(*iIt));
    }
  }
}


void FloatToFixed::readAllLocalMetadata(Module &m, SmallPtrSetImpl<Value *> &res)
{
  for (Function &f: m.functions()) {
    SmallPtrSet<Value*, 32> t;
    readLocalMetadata(f, t);
    res.insert(t.begin(), t.end());

    /* Otherwise dce pass ignore the function
     * (removed also where it's not required) */
    f.removeFnAttr(Attribute::OptimizeNone);
  }
}


bool FloatToFixed::parseMetaData(SmallPtrSetImpl<Value *> &variables, FPType *fpInfo, Value *instr)
{
  ValueInfo vi;

  vi.isBacktrackingNode = false;
  vi.fixpTypeRootDistance = 0;
  
  if (!instr->getType()->isVoidTy()) {
    assert(!(fullyUnwrapPointerOrArrayType(instr->getType())->isStructTy()) && "input info / actual type mismatch");
    vi.fixpType = FixedPointType(fpInfo);
  } else {
    vi.fixpType = FixedPointType();
  }

  variables.insert(instr);
  *valueInfo(instr) = vi;

  return true;
}


bool FloatToFixed::parseStructMetaData(SmallPtrSetImpl<Value *> &variables, StructInfo *fpInfo, Value *instr)
{
  ValueInfo vi;

  vi.isBacktrackingNode = false;
  vi.fixpTypeRootDistance = 0;
  
  if (!instr->getType()->isVoidTy()) {
    assert(fullyUnwrapPointerOrArrayType(instr->getType())->isStructTy() && "input info / actual type mismatch");
    int enableConversion = 0;
    vi.fixpType = FixedPointType::get(fpInfo, &enableConversion);
    if (enableConversion == 0)
      return false;
  } else {
    vi.fixpType = FixedPointType();
  }
  
  variables.insert(instr);
  *valueInfo(instr) = vi;
  
  return true;
}


void FloatToFixed::removeNoFloatTy(SmallPtrSetImpl<Value *> &res)
{
  for (auto it: res) {
    Type *ty;

    AllocaInst *alloca;
    GlobalVariable *global;
    if ((alloca = dyn_cast<AllocaInst>(it))) {
      ty = alloca->getAllocatedType();
    } else if ((global = dyn_cast<GlobalVariable>(it))) {
      ty = global->getType();
    } else {
      LLVM_DEBUG(dbgs() << "annotated instruction " << *it <<
        " not an alloca or a global, ignored\n");
      res.erase(it);
      continue;
    }

    while (ty->isArrayTy() || ty->isPointerTy()) {
      if (ty->isPointerTy())
        ty = ty->getPointerElementType();
      else
        ty = ty->getArrayElementType();
    }
    if (!ty->isFloatingPointTy()) {
      LLVM_DEBUG(dbgs() << "annotated instruction " << *it << " does not allocate a"
        " kind of float; ignored\n");
      res.erase(it);
    }
  }
}

void FloatToFixed::printAnnotatedObj(Module &m)
{
  SmallPtrSet<Value*, 32> res;

  /*readGlobalMetadata(m, res, true);
  errs() << "Annotated Function: \n";
  if(!res.empty())
  {
    for (auto it : res)
    {
      errs() << " -> " << *it << "\n";
    }
    errs() << "\n";
  }*/

  res.clear();
  readGlobalMetadata(m, res);
  errs() << "Global Set: \n";
  if(!res.empty())
  {
    for (auto it : res)
    {
      errs() << " -> " << *it << "\n";
    }
    errs() << "\n";
  }

  for (auto fIt=m.begin() , fItEnd=m.end() ; fIt!=fItEnd ; fIt++)
  {
    Function &f = *fIt;
    errs().write_escaped(f.getName()) << " : ";
    res.clear();
    readLocalMetadata(f, res);
    if(!res.empty())
    {
      errs() << "\nLocal Set: \n";
      for (auto it : res)
      {
        errs() << " -> " << *it << "\n";
      }
    }
    errs() << "\n";
  }

}

