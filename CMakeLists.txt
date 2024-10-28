cmake_minimum_required(VERSION 3.10)
project(cross)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_executable(test_env tests/test_env/main.cc)
add_library(testdll_env SHARED tests/test_env/dllmain.cc)


