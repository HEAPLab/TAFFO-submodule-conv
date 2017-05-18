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
  printAnnotatedObj(m);
  
  auto roots = readAllLocalAnnotations(m);
  for (Value *v: roots) {
    auto vals = buildConversionQueueForRootValue(v);
    outs() << "root from ";
    v->print(outs());
    outs() << "\n";
    for (Value *val: vals) {
      val->print(outs());
      outs() << "\n";
    }
    outs() << "\n\n";
    
    performConversion(m, vals);
  }
  
  return true;
}


std::vector<Value*> FloatToFixed::buildConversionQueueForRootValue(Value *val)
{
  std::vector<Value*> queue;
  size_t next = 0;
  queue.push_back(val);
  
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



