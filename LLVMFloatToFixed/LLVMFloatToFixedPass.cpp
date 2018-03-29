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

//#define LOG_BACKTRACK


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
  DEBUG_WITH_TYPE(DEBUG_ANNOTATION, printAnnotatedObj(m));

  llvm::SmallPtrSet<llvm::Value *, 32> local;
  llvm::SmallPtrSet<llvm::Value *, 32> global;
  readAllLocalAnnotations(m, local);
  readGlobalAnnotations(m, global);
  
  std::vector<Value*> rootsa(local.begin(), local.end());
  rootsa.insert(rootsa.begin(), global.begin(), global.end());
  AnnotationCount = rootsa.size();

  std::vector<Value*> vals;
  buildConversionQueueForRootValues(rootsa, vals);

  if (vals.size() < 1000) {
  DEBUG(errs() << "conversion queue:\n";
        for (Value *val: vals) {
          errs() << "bt=" << info[val].isBacktrackingNode << " ";
          errs() << "[";
            for (Value *rootv: info[val].roots) {
              rootv->print(errs());
              errs() << ' ';
            }
            errs() << "] ";
            val->print(errs());
          errs() << "\n";
        }
        errs() << "\n\n";);
  } else {
    DEBUG(errs() << "not printing the conversion queue because it exceeds 1000 items");
  }
  ConversionCount = vals.size();

  performConversion(m, vals);

  cleanup(vals);

  return true;
}


bool FloatToFixed::isFloatType(Type *srct)
{
  if (srct->isPointerTy()) {
    return isFloatType(srct->getPointerElementType());
  } else if (srct->isArrayTy()) {
    return isFloatType(srct->getArrayElementType());
  } else if (srct->isFloatingPointTy()) {
    return true;
  }
  return false;
}


void FloatToFixed::buildConversionQueueForRootValues(
  const ArrayRef<Value*>& val,
  std::vector<Value*>& queue)
{
  queue.insert(queue.begin(), val.begin(), val.end());
  for (auto i = queue.begin(); i != queue.end(); i++) {
    info[*i].isRoot = true;
  }

  size_t prevQueueSize = 0;
  while (prevQueueSize < queue.size()) {
    dbgs() << "***** buildConversionQueueForRootValues iter " << prevQueueSize << " < " << queue.size() << "\n";
    prevQueueSize = queue.size();

    size_t next = 0;
    while (next < queue.size()) {
      Value *v = queue.at(next);

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
        
        if (info[v].isBacktrackingNode) {
          info[u].isBacktrackingNode = true;
        }
      }
      next++;
    }
    
    next = queue.size();
    for (next = queue.size(); next != 0; next--) {
      Value *v = queue.at(next-1);
      if (!(info[v].isBacktrackingNode))
        continue;
      
      Instruction *inst = dyn_cast<Instruction>(v);
      if (!inst)
        continue;
      
      #ifdef LOG_BACKTRACK
      dbgs() << "BACKTRACK ";
      v->print(dbgs());
      dbgs() << "\n";
      #endif
      
      for (Value *u: inst->operands()) {
        #ifdef LOG_BACKTRACK
        dbgs() << " - ";
        u->print(dbgs());
        #endif
        
        if (!isFloatType(u->getType())) {
          #ifdef LOG_BACKTRACK
          dbgs() << " not a float\n";
          #endif
          continue;
        }
        info[v].isRoot = false;
        
        info[u].isBacktrackingNode = true;
        
        bool alreadyIn = false;
        for (int i=0; i<queue.size() && !alreadyIn;) {
          if (queue[i] == u) {
            if (i < next)
              alreadyIn = true;
            else
              queue.erase(queue.begin() + i);
          } else {
            i++;
          }
        }
        if (alreadyIn) {
          #ifdef LOG_BACKTRACK
          dbgs() << " already in\n";
          #endif
          continue;
        }
        info[u].isRoot = true;
        
        #ifdef LOG_BACKTRACK
        dbgs() << "  enqueued\n";
        #endif
        queue.insert(queue.begin()+next-1, u);
        next++;
      }
    }
  }
  
  for (Value *v: queue) {
    if (info[v].isRoot) {
      info[v].roots = {v};
    }
    
    SmallPtrSet<Value*, 5> newroots = info[v].roots;
    for (Value *u: v->users()) {
      auto oldrootsi = info.find(u);
      if (oldrootsi == info.end())
        continue;
      
      auto oldroots = oldrootsi->getSecond().roots;
      SmallPtrSet<Value*, 5> merge(newroots);
      merge.insert(oldroots.begin(), oldroots.end());
      info[u].roots = merge;
    }
  }
}


bool potentiallyUsesMemory(Value *val)
{
  if (!isa<Instruction>(val))
    return false;
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


void FloatToFixed::cleanup(const std::vector<Value*>& q)
{
  std::vector<Value*> roots;
  for (Value *v: q) {
    if (info[v].isRoot == true)
      roots.push_back(v);
  }
  
  DenseMap<Value*, bool> isrootok;
  for (Value *root: roots)
    isrootok[root] = true;

  for (Value *qi: q) {
    Value *cqi = operandPool[qi];
    assert(cqi && "every value should have been processed at this point!!");
    if (cqi == ConversionError) {
      if (!potentiallyUsesMemory(qi)) {
        continue;
      }
      DEBUG(qi->print(errs());
            errs() << " not converted; invalidates roots ");
      const auto& rootsaffected = info[qi].roots;
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
    const auto& roots = info[v].roots;

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


