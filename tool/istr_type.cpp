#include <iostream>
#include <string>
#include <unordered_map>
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
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


int main(int argc, char *argv[])
{
  cl::ParseCommandLineOptions(argc, argv);

  LLVMContext c;
  SMDiagnostic Err;
  std::unique_ptr<Module> m = parseIRFile(InputFilename, Err, c);

  std::map<std::string, int> stat;

  if (Verbose) {
    std::cerr << "Successfully read Module:" << std::endl;
    std::cerr << " Name: " << m.get()->getName().str() << std::endl;
    std::cerr << " Target triple: " << m->getTargetTriple() << std::endl;
  }

  bool eval = false;
  int ninstr = 0;

  for (auto iter1 = m->getFunctionList().begin();
       iter1 != m->getFunctionList().end(); iter1++) {
    Function &f = *iter1;
    
    if (f.getName() != "main")
      continue;
    if (Verbose)
      std::cerr << " Function: " << f.getName().str() << std::endl;
    
    for (auto iter2 = f.getBasicBlockList().begin();
         iter2 != f.getBasicBlockList().end(); iter2++) {
      BasicBlock &bb = *iter2;
      
      for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) {
        Instruction &inst = *iter3;

        if(isa<CallInst>(inst)) {
          Value *opnd = inst.getOperand(0);
          if (opnd->getName() == "polybench_timer_start") {
            eval = true;
          } else if (opnd->getName() == "polybench_timer_stop") {
            eval = false;
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
        }
      }
    }
  }

  std::cout << "* " << ninstr << std::endl;
  for (auto it = stat.begin(); it != stat.end(); it++) {
    std::cout << it->first << " " << it->second;
    std::cout << std::endl;
  }

  return 0;
}
