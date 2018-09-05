#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <llvm/Transforms/Utils/Cloning.h>
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
  propagateCall(vals, global);
  optimizeFixedPointTypes(vals);

  DEBUG(printConversionQueue(vals));

  ConversionCount = vals.size();

  performConversion(m, vals);

  cleanup(vals);

  return true;
}


bool flttofix::isFloatType(Type *srct)
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


void FloatToFixed::optimizeFixedPointTypes(std::vector<Value*>& queue)
{
  for (Value *v: queue) {
    if (StoreInst *store = dyn_cast<StoreInst>(v)) {
      if (std::find(queue.begin(), queue.end(), store->getPointerOperand()) != queue.end() &&
          std::find(queue.begin(), queue.end(), store->getValueOperand()) != queue.end())
        fixPType(store->getValueOperand()) = fixPType(store->getPointerOperand());
    }
  }
}


void FloatToFixed::buildConversionQueueForRootValues(
  const ArrayRef<Value*>& val,
  std::vector<Value*>& queue)
{
  queue.insert(queue.begin(), val.begin(), val.end());
  for (auto i = queue.begin(); i != queue.end(); i++) {
    info[*i].isRoot = true;
  }
  
  auto completeInfo = [this](Value *v, Value *u) {
    ValueInfo vinfo = info[v];
    ValueInfo &uinfo = info[u];
    uinfo.origType = u->getType();
    if (uinfo.fixpTypeRootDistance > std::max(vinfo.fixpTypeRootDistance, vinfo.fixpTypeRootDistance+1)) {
      uinfo.fixpType = vinfo.fixpType;
      uinfo.fixpTypeRootDistance = std::max(vinfo.fixpTypeRootDistance, vinfo.fixpTypeRootDistance+1);
    }
  };

  size_t prevQueueSize = 0;
  while (prevQueueSize < queue.size()) {
    DEBUG(dbgs() << "***** buildConversionQueueForRootValues iter " << prevQueueSize << " < " << queue.size() << "\n";);
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
        completeInfo(v, u);
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
        if (!alreadyIn) {
          info[u].isRoot = true;
          #ifdef LOG_BACKTRACK
          dbgs() << "  enqueued\n";
          #endif
          queue.insert(queue.begin()+next-1, u);
          next++;
        } else {
          #ifdef LOG_BACKTRACK
          dbgs() << " already in\n";
          #endif
        }
        
        completeInfo(v, u);
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
      if (!info.count(u))
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

  auto clear = [&] (bool (*toDelete) (const Instruction &Y)) {
    for (Value *v: q) {
      Instruction *i = dyn_cast<Instruction>(v);
      if (!i || (!toDelete(*i)))
        continue;
      const auto &roots = info[v].roots;
    
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
      if (allok) {
        if (!i->use_empty())
          i->replaceAllUsesWith(UndefValue::get(i->getType()));
        i->eraseFromParent();
      }
    }
  };
  
  clear(isa<StoreInst>);
  clear(isa<CallInst>);
  clear(isa<InvokeInst>);
  clear(isa<ReturnInst>);
  clear(isa<BranchInst>);
}


void FloatToFixed::propagateCall(std::vector<Value *> &vals, llvm::SmallPtrSetImpl<llvm::Value *> &global)
{
  for (int i=0; i < vals.size(); i++) {
    if (isa<CallInst>(vals[i]) || isa<InvokeInst>(vals[i])) {
      CallSite *call = new CallSite(vals[i]);
      if (Function *newF = createFixFun(call)){
        Function *oldF = call->getCalledFunction();
        
        DEBUG(dbgs() << "Converting function " << oldF->getName() << " : " << *oldF->getType()
                     << " into " << newF->getName() << " : " << *newF->getType() << "\n");
        
        ValueToValueMapTy mapArgs; // Create Val2Val mapping and clone function
        Function::arg_iterator newIt = newF->arg_begin();
        Function::arg_iterator oldIt = oldF->arg_begin();
        for (; oldIt != oldF->arg_end() ; oldIt++, newIt++) {
          newIt->setName(oldIt->getName());
          mapArgs.insert(std::make_pair(oldIt, newIt));
        }
        SmallVector<ReturnInst*,100> returns;
        CloneFunctionInto(newF, oldF, mapArgs, true, returns);
        
        std::vector<Value*> roots; //propagate fixp conversion
        oldIt = oldF->arg_begin();
        newIt = newF->arg_begin();
        for (int i=0; oldIt != oldF->arg_end() ; oldIt++, newIt++,i++) {
          if (oldIt->getType() != newIt->getType()){
            // Mark the alloca used for the argument (in O0 opt lvl)
            info[newIt->user_begin()->getOperand(1)] = info[call->getInstruction()->getOperand(i)];
            roots.push_back(newIt->user_begin()->getOperand(1));
            
            std::string tmpstore; //append fixp info to arg name
            raw_string_ostream tmp(tmpstore);
            tmp << fixPType(newIt->user_begin()->getOperand(1));
            newIt->setName(newIt->getName() + "." + tmp.str());
          }
        }
  
        // If return a float, all the return inst should have a fix point (independently from the conv queue)
        if (oldF->getReturnType()->isFloatingPointTy()) {
          for (ReturnInst *v : returns) {
            roots.push_back(v);
            info[v].fixpType = info[call->getInstruction()].fixpType;
            info[v].fixpTypeRootDistance = 0;
          }
        }
        std::vector<Value*> newVals(global.begin(), global.end());
        buildConversionQueueForRootValues(roots, newVals);
        
        for (Value *val : newVals){
          if (Instruction *inst = dyn_cast<Instruction>(val)) {
            if (inst->getFunction()==newF){
              vals.push_back(val);
            }
          }
        }
      }
    }
  }
}


Function* FloatToFixed::createFixFun(CallSite* call)
{
  Function *oldF = call->getCalledFunction();
  if(isSpecialFunction(oldF))
    return nullptr;
  
  std::vector<Type*> typeArgs;
  std::vector<std::pair<int, FixedPointType>> fixArgs; //for match already converted function
  
  if(isFloatType(oldF->getReturnType())) //ret value in signature
    fixArgs.push_back(std::pair<int, FixedPointType>(-1, info[call->getInstruction()].fixpType));
  
  int i=0;
  for (auto arg = call->arg_begin(); arg != call->arg_end(); arg++,i++) { //detect fix argument
    Value *v = dyn_cast<Value>(arg);
    Type* newTy;
    if (hasInfo(v)) {
      fixArgs.push_back(std::pair<int, FixedPointType>(i,info[v].fixpType));
      newTy = getLLVMFixedPointTypeForFloatValue(v);
    } else {
      newTy = v->getType();
    }
    typeArgs.push_back(newTy);
  }
  
  Function *newF;
  FunctionType *newFunTy = FunctionType::get(
      isFloatType(oldF->getReturnType()) ?
      getLLVMFixedPointTypeForFloatValue(call->getInstruction()) :
      oldF->getReturnType(),
      typeArgs, oldF->isVarArg());
  
  for (FunInfo f : functionPool[oldF]) { //check if is previously converted
    if (f.fixArgs == fixArgs) {
      newF = f.newFun;
      DEBUG(dbgs() << *(call->getInstruction()) <<  " use already converted function : " <<
                   newF->getName() << " " << *newF->getType() << "\n";);
      return nullptr;
    }
  }
  
  newF = Function::Create(newFunTy, oldF->getLinkage(), oldF->getName() + "_fixp", oldF->getParent());
  FunInfo funInfo; //add to pool
  funInfo.newFun = newF;
  funInfo.fixArgs = fixArgs;
  functionPool[oldF].push_back(funInfo);
  FunctionCreated++;
  return newF;
}


void FloatToFixed::printConversionQueue(std::vector<Value*> vals)
{
  if (vals.size() < 1000) {
    errs() << "conversion queue:\n";
                  for (Value *val: vals) {
                    errs() << "bt=" << info[val].isBacktrackingNode << " ";
                    errs() << info[val].fixpType << " ";
                    errs() << "[";
                    for (Value *rootv: info[val].roots) {
                      rootv->print(errs());
                      errs() << ' ';
                    }
                    errs() << "] ";
                    val->print(errs());
                    errs() << "\n";
                  }
                  errs() << "\n\n";
  } else {
    errs() << "not printing the conversion queue because it exceeds 1000 items";
  }
}


