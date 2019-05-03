#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <llvm/Transforms/Utils/ValueMapper.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "LLVMFloatToFixedPass.h"
#include "TypeUtils.h"

//#define LOG_BACKTRACK


using namespace llvm;
using namespace flttofix;
using namespace taffo;


char FloatToFixed::ID = 0;

static RegisterPass<FloatToFixed> X(
  "flttofix",
  "Floating Point to Fixed Point conversion pass",
  true /* Does not only look at CFG */,
  true /* Optimization Pass */);


void FloatToFixed::getAnalysisUsage(llvm::AnalysisUsage &au) const
{
  au.addRequiredTransitive<LoopInfoWrapperPass>();
  au.setPreservesAll();
}


bool FloatToFixed::runOnModule(Module &m)
{
  DEBUG_WITH_TYPE(DEBUG_ANNOTATION, printAnnotatedObj(m));

  llvm::SmallPtrSet<llvm::Value *, 32> local;
  llvm::SmallPtrSet<llvm::Value *, 32> global;
  readAllLocalMetadata(m, local);
  readGlobalMetadata(m, global);

  std::vector<Value*> vals(local.begin(), local.end());
  vals.insert(vals.begin(), global.begin(), global.end());
  MetadataCount = vals.size();

  propagateCall(vals, global);
  sortQueue(vals);
  LLVM_DEBUG(printConversionQueue(vals));
  ConversionCount = vals.size();

  performConversion(m, vals);
  cleanup(vals);

  return true;
}


int FloatToFixed::getLoopNestingLevelOfValue(llvm::Value *v)
{
  Instruction *inst = dyn_cast<Instruction>(v);
  if (!inst)
    return -1;

  Function *fun = inst->getFunction();
  LoopInfo &li = this->getAnalysis<LoopInfoWrapperPass>(*fun).getLoopInfo();
  BasicBlock *bb = inst->getParent();
  return li.getLoopDepth(bb);
}


void FloatToFixed::optimizeFixedPointTypes(std::vector<Value*>& queue)
{
  for (Value *v: queue) {
    if (StoreInst *store = dyn_cast<StoreInst>(v)) {
      if (std::find(queue.begin(), queue.end(), store->getPointerOperand()) != queue.end() &&
          std::find(queue.begin(), queue.end(), store->getValueOperand()) != queue.end()) {
        if (isa<CallInst>(store->getValueOperand())) {
          /* prevent function conversion type mismatches later */
          continue;
        }
        fixPType(store->getValueOperand()) = fixPType(store->getPointerOperand());
      }
    }
  }
}


void FloatToFixed::buildConversionQueueForRootValues(
  const ArrayRef<Value*>& val,
  std::vector<Value*>& queue)
{
  queue.insert(queue.begin(), val.begin(), val.end());
  for (auto i = queue.begin(); i != queue.end(); i++) {
    valueInfo(*i)->isRoot = true;
  }

  auto completeInfo = [this](Value *v, Value *u) {
    ValueInfo vinfo = *valueInfo(v);
    ValueInfo &uinfo = *valueInfo(u);
    uinfo.origType = u->getType();
    if (uinfo.fixpTypeRootDistance > std::max(vinfo.fixpTypeRootDistance, vinfo.fixpTypeRootDistance+1)) {
      uinfo.fixpType = vinfo.fixpType;
      uinfo.fixpTypeRootDistance = std::max(vinfo.fixpTypeRootDistance, vinfo.fixpTypeRootDistance+1);
    }
  };

  size_t prevQueueSize = 0;
  while (prevQueueSize < queue.size()) {
    LLVM_DEBUG(dbgs() << "***** buildConversionQueueForRootValues iter " << prevQueueSize << " < " << queue.size() << "\n";);
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

        if (valueInfo(v)->isBacktrackingNode) {
          valueInfo(u)->isBacktrackingNode = true;
        }
        completeInfo(v, u);
      }
      next++;
    }

    next = queue.size();
    for (next = queue.size(); next != 0; next--) {
      Value *v = queue.at(next-1);
      if (!(valueInfo(v)->isBacktrackingNode))
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
        if (!isa<User>(u) && !isa<Argument>(u)) {
          #ifdef LOG_BACKTRACK
          dbgs() << " - " ;
          u->printAsOperand(dbgs());
          dbgs() << " not a User or an Argument\n";
          #endif
          continue;
        }

        if (isa<Function>(u) || isa<BlockAddress>(u)) {
          #ifdef LOG_BACKTRACK
          dbgs() << " - " ;
          u->printAsOperand(dbgs());
          dbgs() << " is a function/block address\n";
          #endif
          continue;
        }

        #ifdef LOG_BACKTRACK
        dbgs() << " - " << *u;
        #endif

        if (!isFloatType(u->getType())) {
          #ifdef LOG_BACKTRACK
          dbgs() << " not a float\n";
          #endif
          continue;
        }
        valueInfo(v)->isRoot = false;

        valueInfo(u)->isBacktrackingNode = true;

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
          valueInfo(u)->isRoot = true;
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
    if (valueInfo(v)->isRoot) {
      valueInfo(v)->roots = {v};
    }

    SmallPtrSet<Value*, 5> newroots = valueInfo(v)->roots;
    for (Value *u: v->users()) {
      auto oldrootsi = info.find(u);
      if (!info.count(u))
        continue;

      auto oldroots = oldrootsi->getSecond()->roots;
      SmallPtrSet<Value*, 5> merge(newroots);
      merge.insert(oldroots.begin(), oldroots.end());
      valueInfo(u)->roots = merge;
    }
  }
}


void FloatToFixed::sortQueue(std::vector<llvm::Value *> &vals)
{
  size_t next = 0;
  while (next < vals.size()) {
    Value *v = vals.at(next);
    SmallPtrSet<Value*, 5> roots;
    for (Value *oldroot: valueInfo(v)->roots) {
      if (valueInfo(oldroot)->roots.empty())
        roots.insert(oldroot);
    }
    valueInfo(v)->roots.clear();
    valueInfo(v)->roots.insert(roots.begin(), roots.end());
    if (roots.empty()) {
      roots.insert(v);
    }

    for (auto *u: v->users()) {
      /* Insert u at the end of the queue.
       * If u exists already in the queue, *move* it to the end instead. */
      for (int i=0; i<vals.size();) {
        if (vals[i] == u) {
          vals.erase(vals.begin() + i);
          if (i < next)
            next--;
        } else {
          i++;
        }
      }

      if (Instruction *inst = dyn_cast<Instruction>(u)) {
        if (inst->getMetadata(INPUT_INFO_METADATA) || inst->getMetadata(STRUCT_INFO_METADATA)) {
          if (!hasInfo(u)) {
            DEBUG(dbgs() << "[WARNING] Find Value " << *u << " without fixp format!\n");
          }
        } else {
          DEBUG(dbgs() << "[WARNING] Find Value " << *u << " without TAFFO info!\n");
          continue;
        }

      } else if (GlobalObject *go = dyn_cast<GlobalObject>(u)) {
        if (go->getMetadata(INPUT_INFO_METADATA) || go->getMetadata(STRUCT_INFO_METADATA)) {
          if (!hasInfo(u)) {
            DEBUG(dbgs() << "[WARNING] Find GlobalObj " << *u << " without fixp format!\n");
          }
        } else {
          DEBUG(dbgs() << "[WARNING] Find GlobalObj " << *u << " without TAFFO info!\n");
          continue;
        }
      }

      vals.push_back(u);
      valueInfo(u)->roots.insert(roots.begin(), roots.end());
    }
    next++;
  }

  for (Value *v: vals) {
    SmallPtrSetImpl<Value *> &roots = valueInfo(v)->roots;
    if (roots.empty()) {
      valueInfo(v)->isRoot = true;
      if (isa<Instruction>(v) && !isa<AllocaInst>(v)) {
        /* non-alloca roots must have been generated by backtracking */
        valueInfo(v)->isBacktrackingNode = true;
      }
      roots.insert(v);
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
    if (valueInfo(v)->isRoot == true)
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
      LLVM_DEBUG(qi->print(errs());
            if (Instruction *i = dyn_cast<Instruction>(qi))
              errs() << " in function " << i->getFunction()->getName();
            errs() << " not converted; invalidates roots ");
      const auto& rootsaffected = valueInfo(qi)->roots;
      for (Value *root: rootsaffected) {
        isrootok[root] = false;
        LLVM_DEBUG(root->print(errs()));
      }
      LLVM_DEBUG(errs() << '\n');
    }
  }

  std::vector<Instruction *> toErase;

  auto clear = [&] (bool (*toDelete) (const Instruction &Y)) {
    for (Value *v: q) {
      Instruction *i = dyn_cast<Instruction>(v);
      if (!i || (!toDelete(*i)))
        continue;
      const auto &roots = valueInfo(v)->roots;

      bool allok = true;
      for (Value *root: roots) {
        if (!isrootok[root]) {
          LLVM_DEBUG(i->print(errs());
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
        toErase.push_back(i);
      }
    }
  };

  clear(isa<StoreInst>);
  clear(isa<CallInst>);
  clear(isa<InvokeInst>);
  clear(isa<BranchInst>);

  for (Instruction *v: toErase) {
    v->eraseFromParent();
  }
}


void FloatToFixed::propagateCall(std::vector<Value *> &vals, llvm::SmallPtrSetImpl<llvm::Value *> &global)
{
  for (int i=0; i < vals.size(); i++) {
    Value *valsi = vals[i];
    if (isa<CallInst>(valsi) || isa<InvokeInst>(valsi)) {
      CallSite *call = new CallSite(valsi);
      if (Function *newF = createFixFun(call)) {
        Function *oldF = call->getCalledFunction();

        LLVM_DEBUG(dbgs() << "Converting function " << oldF->getName() << " : " << *oldF->getType()
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

        std::vector<Value*> newVals; //propagate fixp conversion
        oldIt = oldF->arg_begin();
        newIt = newF->arg_begin();
        for (int i=0; oldIt != oldF->arg_end() ; oldIt++, newIt++,i++) {
          if (oldIt->getType() != newIt->getType()){
            FixedPointType fixtype = valueInfo(call->getInstruction()->getOperand(i))->fixpType;

            // Mark the alloca used for the argument (in O0 opt lvl)
            valueInfo(newIt->user_begin()->getOperand(1))->fixpType = fixtype;
            valueInfo(newIt->user_begin()->getOperand(1))->fixpTypeRootDistance = 0;
            newVals.push_back(newIt->user_begin()->getOperand(1));

            // Mark the argument itself
            valueInfo(newIt)->fixpType = fixtype;
            valueInfo(newIt)->fixpTypeRootDistance = 0;

            //append fixp info to arg name
            newIt->setName(newIt->getName() + "." + fixtype.toString());
          }
        }

        // If return a float, all the return inst should have a fix point (independently from the conv queue)
        if (oldF->getReturnType()->isFloatingPointTy()) {
          for (ReturnInst *v : returns) {
            newVals.push_back(v);
            valueInfo(v)->fixpType = valueInfo(call->getInstruction())->fixpType;
            valueInfo(v)->fixpTypeRootDistance = 0;
          }
        }

        newVals.insert(newVals.begin(), global.begin(), global.end());
        SmallPtrSet<Value*, 32> localFix;
        readLocalMetadata(*newF, localFix);
        newVals.insert(newVals.begin(), localFix.begin(), localFix.end());

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
  assert(oldF && "bitcasted function pointers and such not handled atm");
  if (isSpecialFunction(oldF))
    return nullptr;

  if (!oldF->getMetadata(SOURCE_FUN_METADATA)) {
    DEBUG(dbgs() << "createFixFun: function " << oldF->getName() << " not a clone; ignoring\n");
    return nullptr;
  }

  std::vector<Type*> typeArgs;
  std::vector<std::pair<int, FixedPointType>> fixArgs; //for match already converted function

  std::string suffix;
  if(isFloatType(oldF->getReturnType())) { //ret value in signature
    FixedPointType retValType = valueInfo(call->getInstruction())->fixpType;
    suffix = retValType.toString();
    fixArgs.push_back(std::pair<int, FixedPointType>(-1, retValType));
  } else {
    suffix = "fixp";
  }

  int i=0;
  for (auto arg = oldF->arg_begin(); arg != oldF->arg_end(); arg++, i++) {
    Value *v = dyn_cast<Value>(arg);
    Type* newTy;
    if (hasInfo(v)) {
      fixArgs.push_back(std::pair<int, FixedPointType>(i,valueInfo(v)->fixpType));
      newTy = getLLVMFixedPointTypeForFloatValue(v);
    } else {
      newTy = v->getType();
    }
    typeArgs.push_back(newTy);
  }

  Function *newF = functionPool[oldF]; //check if is previously converted
  if (newF) {
    LLVM_DEBUG(dbgs() << *(call->getInstruction()) <<  " use already converted function : " <<
                 newF->getName() << " " << *newF->getType() << "\n";);
    return nullptr;
  }

  FunctionType *newFunTy = FunctionType::get(
      isFloatType(oldF->getReturnType()) ?
      getLLVMFixedPointTypeForFloatValue(call->getInstruction()) :
      oldF->getReturnType(),
      typeArgs, oldF->isVarArg());

  LLVM_DEBUG({
    dbgs() << "creating function " << oldF->getName() << "_" << suffix << " with types ";
    for (auto pair: fixArgs) {
      dbgs() << "(" << pair.first << ", " << pair.second << ") ";
    }
    dbgs() << "\n";
  });

  newF = Function::Create(newFunTy, oldF->getLinkage(), oldF->getName() + "_" + suffix, oldF->getParent());
  functionPool[oldF] = newF; //add to pool
  FunctionCreated++;
  return newF;
}


void FloatToFixed::printConversionQueue(std::vector<Value*> vals)
{
  if (vals.size() < 1000) {
    errs() << "conversion queue:\n";
                  for (Value *val: vals) {
                    errs() << "bt=" << valueInfo(val)->isBacktrackingNode << " ";
                    errs() << valueInfo(val)->fixpType << " ";
                    errs() << "[";
                    for (Value *rootv: valueInfo(val)->roots) {
                      rootv->print(errs());
                      errs() << ' ';
                    }
                    errs() << "] ";
                    val->print(errs());
                    errs() << "\n";
                  }
                  errs() << "\n\n";
  } else {
    errs() << "not printing the conversion queue because it exceeds 1000 items\n";
  }
}
