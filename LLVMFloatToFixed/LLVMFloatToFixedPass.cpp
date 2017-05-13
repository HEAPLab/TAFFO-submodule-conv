#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;


namespace {

struct FloatToFixed : public ModulePass {
  static char ID;
  
  FloatToFixed(): ModulePass(ID) { }
  bool runOnModule(Module &M) override;
};

}


char FloatToFixed::ID = 0;

static RegisterPass<FloatToFixed> X(
  "flttofix",
  "Floating Point to Fixed Point conversion pass",
  true /* Does not only look at CFG */,
  true /* Optimization Pass */);


bool FloatToFixed::runOnModule(Module &M)
{
  return false;
}
