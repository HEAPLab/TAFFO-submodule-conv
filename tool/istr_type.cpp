#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


cl::OptionCategory IstrFreqOptions("istr_type options");
cl::opt<bool> Verbose("verbose", cl::value_desc("verbose"),
  cl::desc("Enable Verbose Output"),
  cl::init(false));
cl::opt<std::string> InputFilename(cl::Positional, cl::Required,
  cl::desc("<input file>"));


void analyze_function(Function *f, std::unordered_set<Function *>& funcs, std::map<std::string, int>& stat, int &eval, int &ninstr)
{
  if (funcs.find(f) != funcs.end()) {
    if (Verbose)
      std::cerr << "Recursion!" << std::endl;
    return;
  }
  if (Verbose)
    std::cerr << " Function: " << f->getName().str() << std::endl;
  funcs.insert(f);
  
  for (auto iter2 = f->getBasicBlockList().begin();
       iter2 != f->getBasicBlockList().end(); iter2++) {
    BasicBlock &bb = *iter2;
    
    for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) {
      Instruction &inst = *iter3;

      CallInst *call = dyn_cast<CallInst>(&inst);
      if(call) {
        Function *opnd = call->getCalledFunction();
        if (opnd->getName() == "polybench_timer_start" ||
            opnd->getName() == "timer_start") {
          eval++;
          continue;
        } else if (opnd->getName() == "polybench_timer_stop" ||
                   opnd->getName() == "timer_stop") {
          eval--;
        } else if (opnd->getIntrinsicID() == Intrinsic::ID::annotation ||
                   opnd->getIntrinsicID() == Intrinsic::ID::var_annotation ||
                   opnd->getIntrinsicID() == Intrinsic::ID::ptr_annotation) {
          continue;
        } else {
          analyze_function(opnd, funcs, stat, eval, ninstr);
        }
      }

      if (!eval)
        continue;
      
      ninstr++;
      stat[inst.getOpcodeName()]++;

      if (isa<AllocaInst>(inst) || isa<LoadInst>(inst) || isa<StoreInst>(inst) || isa<GetElementPtrInst>(inst) ) {
        stat["MemOp"]++;
      } else if (isa<PHINode>(inst) || isa<SelectInst>(inst) || isa<FCmpInst>(inst) || isa<CmpInst>(inst) ) {
        stat["CmpOp"]++;
      } else if (isa<CastInst>(inst)) {
        stat["CastOp"]++;
      } else if (inst.isBinaryOp()) {
        stat["MathOp"]++;
        if (inst.getType()->isFloatingPointTy()) {
          stat["FloatingPointOp"]++;
          if (inst.getOpcode() == Instruction::FMul || inst.getOpcode() == Instruction::FDiv)
            stat["FloatMulDivOp"]++;
        } else
          stat["IntegerOp"]++;
      }
      if (inst.isShift()) {
        stat["Shift"]++;
      }
      
      if (call) {
        std::stringstream stm;
        stm << "call(" << call->getCalledFunction()->getName().str() << ")";
        stat[stm.str()]++;
      }
    }
  }
  
  funcs.erase(f);
}


int main(int argc, char *argv[])
{
  cl::ParseCommandLineOptions(argc, argv);

  LLVMContext c;
  SMDiagnostic Err;
  std::unique_ptr<Module> m = parseIRFile(InputFilename, Err, c);

  if (Verbose) {
    std::cerr << "Successfully read Module:" << std::endl;
    std::cerr << " Name: " << m.get()->getName().str() << std::endl;
    std::cerr << " Target triple: " << m->getTargetTriple() << std::endl;
  }

  int eval = 0;
  int ninstr = 0;
  std::map<std::string, int> stat;
  std::unordered_set<Function *> funcs;
  
  Function *mainfunc = m->getFunction("main");
  if (!mainfunc) {
    std::cout << "No main function found!\n";
  } else {
    analyze_function(mainfunc, funcs, stat, eval, ninstr);
  }

  for (auto iter1 = m->getFunctionList().begin();
       iter1 != m->getFunctionList().end(); iter1++) {
    Function &f = *iter1;
    
    if (f.getName() != "main")
      continue;
    
  }

  std::cout << "* " << ninstr << std::endl;
  for (auto it = stat.begin(); it != stat.end(); it++) {
    std::cout << it->first << " " << it->second;
    std::cout << std::endl;
  }

  return 0;
}
