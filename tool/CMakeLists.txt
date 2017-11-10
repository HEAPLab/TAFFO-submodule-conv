add_executable(istr_type
  istr_type.cpp
)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs support core irreader)

target_link_libraries(istr_type ${llvm_libs})

set_property(TARGET istr_type PROPERTY CXX_STANDARD 11)
set_property(TARGET istr_type PROPERTY CXX_STANDARD_REQUIRED ON)

