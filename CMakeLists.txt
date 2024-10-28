cmake_minimum_required(VERSION 3.10)
project(cross)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOLUTION_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(SOLUTION_INCLUDE_DIR ${SOLUTION_ROOT_DIR}/include/)
set(LOCAL_CMAKE_MODULES_PATH ${SOLUTION_ROOT_DIR}/cmake/modules/)

set(CMAKE_BINARY_DIR ${SOLUTION_ROOT_DIR}/cmake_build/)

LIST(APPEND CMAKE_MODULE_PATH ${LOCAL_CMAKE_MODULES_PATH})
include(global)



# add_executable(test_env tests/test_env/main.cc)
add_library(testdll_env SHARED tests/test_env/dllmain.cc)


