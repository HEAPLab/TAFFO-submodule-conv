add_llvm_library(LLVMFloatToFixed OBJECT BUILDTREE_ONLY
  FixedPointType.cpp
  LLVMFloatToFixedPass.cpp
  MDCollector.cpp
  Conversion.cpp
  ConstantConversion.cpp
  InstructionConversion.cpp
  TAFFOMath.h
  TAFFOMath.cpp
  SinCos.cpp

  ADDITIONAL_HEADERS
  TAFFOMath.h
  SinCos.h  
  FixedPointType.h
  LLVMFloatToFixedPass.h
)

set_property(TARGET obj.LLVMFloatToFixed PROPERTY POSITION_INDEPENDENT_CODE ON)
