add_llvm_library(LLVMFloatToFixed OBJECT BUILDTREE_ONLY
  FixedPointType.cpp
  LLVMFloatToFixedPass.cpp
  MDCollector.cpp
  Conversion.cpp
  ConstantConversion.cpp
  InstructionConversion.cpp

  ADDITIONAL_HEADERS
  FixedPointType.h
  LLVMFloatToFixedPass.h
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_property(TARGET obj.LLVMFloatToFixed APPEND_STRING PROPERTY
    LINK_FLAGS " -undefined dynamic_lookup")
endif()

set_property(TARGET obj.LLVMFloatToFixed PROPERTY CXX_STANDARD 11)
set_property(TARGET obj.LLVMFloatToFixed PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET obj.LLVMFloatToFixed PROPERTY POSITION_INDEPENDENT_CODE ON)
