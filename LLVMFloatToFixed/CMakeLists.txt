add_llvm_library(LLVMFloatToFixed MODULE
  FixedPointType.h
  FixedPointType.cpp
  LLVMFloatToFixedPass.cpp
  LLVMFloatToFixedPass.h
  MDCollector.cpp
  Conversion.cpp
  ConstantConversion.cpp
  InstructionConversion.cpp
  TAFFOMath.h
  TAFFOMath.cpp
  SinCos.cpp
  SinCos.h  
  PLUGIN_TOOL
  opt
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_property(TARGET LLVMFloatToFixed APPEND_STRING PROPERTY
    LINK_FLAGS " -undefined dynamic_lookup")
endif()

set_property(TARGET LLVMFloatToFixed PROPERTY CXX_STANDARD 11)
set_property(TARGET LLVMFloatToFixed PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(LLVMFloatToFixed TaffoUtils)
