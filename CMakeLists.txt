cmake_minimum_required(VERSION 3.19)
project(ToyC)
set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
FIND_PACKAGE(BISON REQUIRED)
FIND_PACKAGE(FLEX REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(${LLVM_DEFINITIONS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src/)
BISON_TARGET(Parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.hpp)
FLEX_TARGET(Tokens src/tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp)
ADD_FLEX_BISON_DEPENDENCY(Tokens Parser)

#shouldn't use GLOB but whatever
FILE(GLOB_RECURSE sources ${CMAKE_SOURCE_DIR}/src/*.cpp)
message("${sources}")

add_executable(ToyC ${sources} ${BISON_Parser_OUTPUTS} ${FLEX_Tokens_OUTPUTS})

target_include_directories(ToyC PUBLIC ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs core native support mcjit executionengine)
target_link_libraries(ToyC PUBLIC ${llvm_libs})

