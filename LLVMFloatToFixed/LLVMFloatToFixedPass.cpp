#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"

#define N_ANNO_VAR 32
#define NO_FLOAT_ANNO "no_float"

using namespace llvm;


namespace {

struct FloatToFixed : public ModulePass {
  static char ID;

  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(Module &M) override;
  SmallPtrSet<Value*,N_ANNO_VAR> readGlobalAnnotations(Module &m , bool functionAnnotation = false);
  SmallPtrSet<Value*,N_ANNO_VAR> readLocalAnnotations(Function &f);
  bool isValidAnnotation(ConstantExpr *expr);
  SmallPtrSet<Value*,N_ANNO_VAR> removeNoFloatTy(SmallPtrSet<Value*,N_ANNO_VAR> res);
  void printAnnotatedObj(Module &m);
};

}


char FloatToFixed::ID = 0;

static RegisterPass<FloatToFixed> X(
  "flttofix",
  "Floating Point to Fixed Point conversion pass",
  true /* Does not only look at CFG */,
  true /* Optimization Pass */);


bool FloatToFixed::runOnModule(Module &m)
{
  printAnnotatedObj(m);

  return false;
}

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
              if (isValidAnnotation(cast<ConstantExpr>(anno->getOperand(1))))
              {
                variables.insert(expr->getOperand(0));
              }
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
  for (inst_iterator iIt = inst_begin(&f), iItEnd = inst_end(&f); iIt != iItEnd; iIt++)
  {
    if (iIt->getOpcode() == Instruction::Call &&  cast<CallInst>(*iIt).getCalledFunction()->getName() == "llvm.var.annotation")
    {
      if (isValidAnnotation(cast<ConstantExpr>(iIt->getOperand(1))))
      {
        Instruction *var = cast<Instruction>(iIt->getOperand(0));
        variables.insert(var->getOperand(0));
      }
    }
  }
  return removeNoFloatTy(variables);
}


bool FloatToFixed::isValidAnnotation(ConstantExpr *annoPtrInst)
{
  if (annoPtrInst->getOpcode() == Instruction::GetElementPtr)
  {
    if (GlobalVariable *annoContent = dyn_cast<GlobalVariable>(annoPtrInst->getOperand(0)))
    {
      if (ConstantDataSequential *annoStr = dyn_cast<ConstantDataSequential>(annoContent->getInitializer()))
      {
        if (annoStr->isString())
        {
          std::string str = annoStr->getAsString();
          return !str.compare(0,str.length()-1,NO_FLOAT_ANNO) ?  true :  false;
        }
      }
    }
  }
  return false;
}

SmallPtrSet<Value*,N_ANNO_VAR> FloatToFixed::removeNoFloatTy(SmallPtrSet<Value*,N_ANNO_VAR> res)
{
  for(auto it : res)
  {
    if (!dyn_cast<PointerType>(it->getType())->getElementType()->isFloatingPointTy())
    {
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
