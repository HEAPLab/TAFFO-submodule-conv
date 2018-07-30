cmake_minimum_required(VERSION 3.4.3) # like llvm

project(TAFFO-Conversion)

find_package(LLVM REQUIRED CONFIG)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_subdirectory(LLVMFloatToFixed)
add_subdirectory(tool)
