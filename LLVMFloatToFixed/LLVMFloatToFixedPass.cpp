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


char FloatToFixed::ID = 0;

static RegisterPass<FloatToFixed> X(
  "flttofix",
  "Floating Point to Fixed Point conversion pass",
  true /* Does not only look at CFG */,
  true /* Optimization Pass */);


bool FloatToFixed::runOnModule(Module &m)
{
  DEBUG_WITH_TYPE(DEBUG_ANNOTATION,printAnnotatedObj(m));


  auto roots = readAllLocalAnnotations(m);
  std::vector<Value*> rootsa(roots.begin(), roots.end());
  auto vals = buildConversionQueueForRootValues(rootsa);

  DEBUG(errs() << "conversion queue:\n";
        for (Value *val: vals) {
          val->print(outs());
          errs() << "\n";
        }
        errs() << "\n\n";);


  performConversion(m, vals);

  return true;
}


std::vector<Value*> FloatToFixed::buildConversionQueueForRootValues(const ArrayRef<Value*>& val)
{
  std::vector<Value*> queue(val);
  size_t next = 0;

  while (next < queue.size()) {
    Value *v = queue.at(next);
    for (auto *u: v->users()) {
      for (int i=0; i<queue.size();) {
        if (queue[i] == u) {
          queue.erase(queue.begin() + i);
          if (i < next)
            next--;
        } else {
          i++;
        }
      }
      queue.push_back(u);
    }
    next++;
  }

  return queue;
}




