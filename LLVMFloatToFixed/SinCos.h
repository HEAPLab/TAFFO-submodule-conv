#pragma once


namespace flttofix {

bool partialSpecialCallSinCos(
    llvm::Function *oldf, bool &foundRet, flttofix::FixedPointType &fxpret,
    SmallVector<std::pair<BasicBlock *, SmallVector<Value *, 10>>, 3>
        &to_change);


void fixrangeSinCos(FloatToFixed *ref, Function *oldf, FixedPointType &fxparg,
                    FixedPointType &fxpret, Value *arg_value,
                    llvm::IRBuilder<> &builder);

}