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


cl::opt<int> GlobalFracBitsAmt("fixpfracbitsamt", cl::value_desc("bits"),
  cl::desc("Default amount of fractional bits in fixed point numbers"),
  cl::init(16));
cl::opt<int> GlobalBitsAmt("fixpbitsamt", cl::value_desc("bits"),
  cl::desc("Default amount of bits in fixed point numbers"), cl::init(32));


char FloatToFixed::ID = 0;

static RegisterPass<FloatToFixed> X(
  "flttofix",
  "Floating Point to Fixed Point conversion pass",
  true /* Does not only look at CFG */,
  true /* Optimization Pass */);


bool FloatToFixed::runOnModule(Module &m)
{
  DEBUG_WITH_TYPE(DEBUG_ANNOTATION, printAnnotatedObj(m));
  fracBitsAmt = GlobalFracBitsAmt;
  bitsAmt = GlobalBitsAmt;

  auto roots = readAllLocalAnnotations(m);
  std::vector<Value*> rootsa(roots.begin(), roots.end());
  std::vector<Value*> vals;
  DenseMap<Value*, SmallPtrSet<Value*, 5>> itemtoroot;
  buildConversionQueueForRootValues(rootsa, vals, itemtoroot);

  DEBUG(errs() << "conversion queue:\n";
        for (Value *val: vals) {
          errs() << "[";
            for (Value *rootv: itemtoroot[val]) {
              rootv->print(errs());
              errs() << ' ';
            }
            errs() << "] ";
            val->print(errs());
          errs() << "\n";
        }
        errs() << "\n\n";);
  ConversionCount = vals.size();

  performConversion(m, vals);

  return true;
}


void FloatToFixed::buildConversionQueueForRootValues(
  const ArrayRef<Value*>& val,
  std::vector<Value*>& queue,
  DenseMap<Value*, SmallPtrSet<Value*, 5>>& itemtoroot)
{
  queue.insert(queue.begin(), val.begin(), val.end());
  for (auto i = queue.begin(); i != queue.end(); i++) {
    itemtoroot[*i] = {*i};
  }

  size_t next = 0;
  while (next < queue.size()) {
    Value *v = queue.at(next);
    SmallPtrSet<Value*, 5>& newroots = itemtoroot[v];
    
    for (auto *u: v->users()) {
      /* Insert u at the end of the queue.
       * If u exists already in the queue, *move* it to the end instead. */
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
      
      auto oldrootsi = itemtoroot.find(u);
      if (oldrootsi == itemtoroot.end()) {
        itemtoroot[u] = newroots;
      } else {
        SmallPtrSet<Value*, 5> merge(newroots);
        merge.insert(oldrootsi->getSecond().begin(), oldrootsi->getSecond().end());
        itemtoroot[u] = merge;
      }
    }
    next++;
  }
}




