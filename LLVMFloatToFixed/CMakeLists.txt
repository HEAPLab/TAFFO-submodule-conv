add_llvm_loadable_module(LLVMFloatToFixed
  LLVMFloatToFixedPass.cpp
  LLVMFloatToFixedPass.h
  Annotations.cpp
  Conversion.cpp
  ConstantConversion.cpp
  InstructionConversion.cpp
  
  PLUGIN_TOOL
  opt
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_property(TARGET LLVMFloatToFixed APPEND_STRING PROPERTY
    LINK_FLAGS " -undefined dynamic_lookup")
endif()

set_property(TARGET LLVMFloatToFixed PROPERTY CXX_STANDARD 11)
set_property(TARGET LLVMFloatToFixed PROPERTY CXX_STANDARD_REQUIRED ON)

