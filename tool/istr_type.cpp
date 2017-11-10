#include <iostream>
#include <string>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/raw_ostream.h>

#include <unordered_map>

using namespace llvm;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << "bitcode_filename" << std::endl;
    return 1;
  }

  LLVMContext c;
  SMDiagnostic Err;
  std::unique_ptr<Module> m = parseIRFile(argv[1], Err, c);

  std::map<std::string, int> stat;

  std::cout << "Successfully read Module:" << std::endl;
  std::cout << " Name: " << m.get()->getName().str() << std::endl;
  std::cout << " Target triple: " << m->getTargetTriple() << std::endl;

  bool eval = false;

  for (auto iter1 = m->getFunctionList().begin();
       iter1 != m->getFunctionList().end(); iter1++) {
    Function &f = *iter1;
    if (f.getName() != "main") continue;
    std::cout << " Function: " << f.getName().str() << std::endl;
    for (auto iter2 = f.getBasicBlockList().begin();
         iter2 != f.getBasicBlockList().end(); iter2++) {
      BasicBlock &bb = *iter2;
      //std::cout << "  BasicBlock: " << bb.getName().str() << std::endl;
      for (auto iter3 = bb.begin(); iter3 != bb.end(); iter3++) {
        Instruction &inst = *iter3;
        //std::cout << "   Instruction " << &inst << " : " << inst.getOpcodeName();
        //std:: cout << std::endl;


        if(isa<CallInst>(inst)) {
          Value *opnd = inst.getOperand(0);
          if (opnd->getName() == "polybench_timer_start") {
            eval = true;
          } else if (opnd->getName() == "polybench_timer_stop") {
            eval = false;
          }
        }

        if (!eval) continue;

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


        /*unsigned int  i = 0;
        unsigned int opnt_cnt = inst.getNumOperands();
        for(; i < opnt_cnt; ++i)
        {
          Value *opnd = inst.getOperand(i);
          std::string o;
          //          raw_string_ostream os(o);
          //         opnd->print(os);
          //opnd->printAsOperand(os, true, m);
          if (opnd->hasName()) {
            o = opnd->getName();
            std::cout << " " << o << "," ;
          } else {
            ;std::cout << " ptr" << opnd << ",";
          }
        }
        std:: cout << std::endl;*/


      }
    }
  }

  for (auto it = stat.begin(); it != stat.end(); it++) {
    std::cout << " " << it->first << ":" << it->second;
    std::cout << std::endl;
  }


  return 0;
}
