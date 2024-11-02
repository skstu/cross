cmake_minimum_required(VERSION 3.10)

project(cross)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(CMAKE_SYSTEM_NAME "Windows")
    set(CC C:/msys64/clang64/bin/clang)
    set(CXX C:/msys64/clang64/bin/clang++)
elseif(APPLE)
    set(CMAKE_SYSTEM_NAME "macOS")
elseif(UNIX)
    set(CMAKE_SYSTEM_NAME "Linux")
else()
    set(CMAKE_SYSTEM_NAME "Unknown System")
endif()

message(STATUS ${CMAKE_SYSTEM_NAME})
#[[
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
 message(STATUS "System: Windows")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
 message(STATUS "System: Linux")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
 message(STATUS "System: macOS")
 
else()
 message(STATUS "Unknown system: ${CMAKE_SYSTEM_NAME}")
endif()
]]


set(SOLUTION_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/)
set(SOLUTION_INCLUDE_DIR ${SOLUTION_ROOT_DIR}/include/)
set(LOCAL_CMAKE_MODULES_PATH ${SOLUTION_ROOT_DIR}/cmake/modules/)

set(CMAKE_BINARY_DIR ${SOLUTION_ROOT_DIR}/cmake_build/)

add_definitions(-DUNICODE -D_UNICODE)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;" CACHE STRING "info" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

LIST(APPEND CMAKE_MODULE_PATH ${LOCAL_CMAKE_MODULES_PATH})
include(global)

# add_executable(test_env tests/test_env/main.cc)
add_library(testdll_env SHARED tests/test_env/dllmain.cc)

