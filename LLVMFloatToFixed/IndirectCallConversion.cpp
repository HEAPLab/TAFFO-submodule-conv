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
#include "TypeUtils.h"

using namespace llvm;
using namespace taffo;
using namespace flttofix;

void FloatToFixed::convertIndirectCalls(llvm::Module& m) {
  std::vector<llvm::CallInst *> trampolineCalls;

  for (llvm::Function &curFunction : m) {
    for (auto instructionIt = inst_begin(curFunction); instructionIt != inst_end(curFunction); instructionIt++) {
      if (auto curCallInstruction = llvm::dyn_cast<llvm::CallInst>(&(*instructionIt))) {
        if (curCallInstruction->getMetadata(INDIRECT_METADATA)) {
          trampolineCalls.push_back(curCallInstruction);
        }
      }
    }
  }

  for (auto trampolineCall: trampolineCalls) {
    auto *metadata = dyn_cast<ValueAsMetadata>(*trampolineCall->getMetadata(INDIRECT_METADATA)->op_begin());
    Function *indirectFunction = dyn_cast<Function>(metadata->getValue());

    // FIXME Use a map instead of if, figuring out the type of FloatToFixed::handleKmpcFork
    if (indirectFunction->getName() == "__kmpc_fork_call") {
      handleKmpcFork(trampolineCall, indirectFunction);
    }
  }
}

void FloatToFixed::handleKmpcFork(CallInst *I, Function *indirectFunction) {
  auto calledFunction = cast<CallInst>(I)->getCalledFunction();
  auto entryBlock = &calledFunction->getEntryBlock();

  auto fixpCallInstr = entryBlock->getTerminator()->getPrevNode();
  assert(llvm::isa<llvm::CallInst>(fixpCallInstr) && "expected a CallInst to the outlined function");
  auto fixpCall = cast<CallInst>(fixpCallInstr);

  auto microTaskType = indirectFunction->getArg(2)->getType();
  auto bitcastedMicroTask = ConstantExpr::getBitCast(fixpCall->getCalledFunction(), microTaskType);

  std::vector<Value *> indirectCallArgs = std::vector<Value *>(I->arg_begin(), I->arg_end());
  indirectCallArgs.insert(indirectCallArgs.begin() + 2, bitcastedMicroTask);

  auto indirectCall = CallInst::Create(indirectFunction, indirectCallArgs);
  indirectCall->insertAfter(I);

  cpMetaData(indirectCall, I);

  I->eraseFromParent();
}
