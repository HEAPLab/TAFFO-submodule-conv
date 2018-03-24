#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "LLVMFloatToFixedPass.h"

using namespace llvm;
using namespace flttofix;


SmallPtrSet<Value*,N_ANNO_VAR> FloatToFixed::readGlobalAnnotations(Module &m , bool functionAnnotation)
{
  SmallPtrSet <Value*,N_ANNO_VAR> variables;

  GlobalVariable *globAnnos = m.getGlobalVariable("llvm.global.annotations");

  if (globAnnos != NULL)
  {
    if (ConstantArray *annos = dyn_cast<ConstantArray>(globAnnos->getInitializer()))
    {
      for (unsigned i = 0, n = annos->getNumOperands(); i < n; i++)
      {
        if (ConstantStruct *anno = dyn_cast<ConstantStruct>(annos->getOperand(i)))
        {
          /*struttura expr (operando 0 contiene expr) :
            [OpType] operandi :
            [BitCast] *funzione , [GetElementPtr] *annotazione ,
            [GetElementPtr] *filename , [Int] linea di codice sorgente) */
          if (ConstantExpr *expr = dyn_cast<ConstantExpr>(anno->getOperand(0)))
          {
            if (expr->getOpcode() == Instruction::BitCast && (functionAnnotation ^ !isa<Function>(expr->getOperand(0))) )
            {
              parseAnnotation(variables, cast<ConstantExpr>(anno->getOperand(1)), expr->getOperand(0));
            }
          }
        }
      }
    }
  }
  return functionAnnotation ? variables : removeNoFloatTy(variables);
}


SmallPtrSet<Value*,N_ANNO_VAR> FloatToFixed::readLocalAnnotations(Function &f)
{
  SmallPtrSet <Value*,N_ANNO_VAR> variables;
  for (inst_iterator iIt = inst_begin(&f), iItEnd = inst_end(&f); iIt != iItEnd; iIt++) {
    CallInst *call = dyn_cast<CallInst>(&(*iIt));
    if (!call)
      continue;
    
    if (!call->getCalledFunction())
      continue;
    
    if (call->getCalledFunction()->getName() == "llvm.var.annotation") {
      parseAnnotation(variables, cast<ConstantExpr>(iIt->getOperand(1)), iIt->getOperand(0));
    }
  }
  return removeNoFloatTy(variables);
}


SmallPtrSet<Value*, N_ANNO_VAR> FloatToFixed::readAllLocalAnnotations(Module &m)
{
  SmallPtrSet<Value*, N_ANNO_VAR> res;

  for (Function &f: m.functions()) {
    SmallPtrSet<Value*, N_ANNO_VAR> t = readLocalAnnotations(f);
    res.insert(t.begin(), t.end());
  }
  return res;
}


bool FloatToFixed::parseAnnotation(SmallPtrSet<Value*,N_ANNO_VAR>& variables, ConstantExpr *annoPtrInst, Value *instr)
{
  ValueInfo vi;
  
  if (!(annoPtrInst->getOpcode() == Instruction::GetElementPtr))
    return false;
  GlobalVariable *annoContent = dyn_cast<GlobalVariable>(annoPtrInst->getOperand(0));
  if (!annoContent)
    return false;
  ConstantDataSequential *annoStr = dyn_cast<ConstantDataSequential>(annoContent->getInitializer());
  if (!annoStr)
    return false;
  if (!(annoStr->isString()))
    return false;
  
  std::string str = annoStr->getAsString();
  if (str.compare(0, str.length() - 1, "no_float") == 0)
    vi.isBacktrackingNode = false;
  else if (str.compare(0, str.length() - 1, "force_no_float") == 0)
    vi.isBacktrackingNode = true;
  else
    return false;
  
  variables.insert(instr);
  info[instr] = vi;
  return true;
}


SmallPtrSet<Value*,N_ANNO_VAR> FloatToFixed::removeNoFloatTy(SmallPtrSet<Value*,N_ANNO_VAR> &res)
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
      DEBUG(dbgs() << "annotated instruction " << *it <<
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
      DEBUG(dbgs() << "annotated instruction " << *it << " does not allocate a"
        " kind of float; ignored\n");
      res.erase(it);
    }
  }
  return res;
}

void FloatToFixed::printAnnotatedObj(Module &m)
{
  SmallPtrSet<Value*,N_ANNO_VAR> res;

  res = readGlobalAnnotations(m,true);
  errs() << "Annotated Function: \n";
  if(!res.empty())
  {
    for (auto it : res)
    {
      errs() << " -> " << *it << "\n";
    }
    errs() << "\n";
  }

  res = readGlobalAnnotations(m);
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
    res = readLocalAnnotations(f);
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

