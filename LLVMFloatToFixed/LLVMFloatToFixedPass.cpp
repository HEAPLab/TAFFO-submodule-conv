#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
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

  auto local = readAllLocalAnnotations(m);
  auto global = readGlobalAnnotations(m);
  std::vector<Value*> rootsa(local.begin(), local.end());
  rootsa.insert(rootsa.begin(), global.begin(), global.end());
  AnnotationCount = rootsa.size();
  
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

  DenseMap<Value*, Value*> operandPool;
  performConversion(m, vals, operandPool);
  
  cleanup(operandPool, vals, itemtoroot, rootsa);

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
    SmallPtrSet<Value*, 5> newroots = itemtoroot[v];
    
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


bool potentiallyUsesMemory(Value *val)
{
  if (isa<BitCastInst>(val))
    return false;
  if (CallInst *call = dyn_cast<CallInst>(val)) {
    Function *f = call->getCalledFunction();
    if (!f)
      return true;
    if (f->isIntrinsic()) {
      Intrinsic::ID fiid = f->getIntrinsicID();
      if (fiid == Intrinsic::lifetime_start ||
          fiid == Intrinsic::lifetime_end)
        return false;
    }
    return !f->doesNotAccessMemory();
  }
  return true;
}


void FloatToFixed::cleanup(
  DenseMap<Value*, Value*> convertedPool,
  const std::vector<Value*>& q,
  const DenseMap<Value*, SmallPtrSet<Value*, 5>>& itemtoroot,
  const std::vector<Value*>& roots)
{
  DenseMap<Value*, bool> isrootok;
  for (Value *root: roots)
    isrootok[root] = true;
  
  for (Value *qi: q) {
    Value *cqi = convertedPool[qi];
    assert(cqi || (!isa<Instruction>(qi)) && "every value should have been processed at this point!!");
    if (!cqi)
      continue;
    if (cqi == ConversionError) {
      if (!potentiallyUsesMemory(qi)) {
        continue;
      }
      DEBUG(qi->print(errs());
            errs() << " not converted; invalidates roots ");
      const auto& rootsaffected = itemtoroot.find(qi)->getSecond();
      for (Value *root: rootsaffected) {
        isrootok[root] = false;
        DEBUG(root->print(errs()));
      }
      DEBUG(errs() << '\n');
    }
  }
  
  for (Value *v: q) {
    StoreInst *i = dyn_cast<StoreInst>(v);
    if (!i)
      continue;
    const auto& roots = itemtoroot.find(v)->getSecond();
    
    bool allok = true;
    for (Value *root: roots) {
      if (!isrootok[root]) {
        DEBUG(i->print(errs());
              errs() << " not deleted: involves root ";
              root->print(errs());
              errs() << '\n');
        allok = false;
        break;
      }
    }
    if (allok)
      i->eraseFromParent();
  }
}


