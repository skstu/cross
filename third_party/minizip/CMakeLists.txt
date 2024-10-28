project (minizip C)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES 
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/minizip.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/ioapi.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/iowin32.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/mztools.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/unzip.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/zip.c
${CMAKE_CURRENT_SOURCE_DIR}/../zlib/contrib/minizip/miniunz.c
)


if(MSVC)
    add_compile_options(/wd4996 /wd4267)
endif()


add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} 
PRIVATE
${SOLUTION_INCLUDE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(MSVC)
MSVC_LINK_FLAGS_SET_CONSOLE(${PROJECT_NAME})
if(ENABLE_MSVC_CRT_MT)
MSVC_CRT_SET(${PROJECT_NAME} "/MT")
endif(ENABLE_MSVC_CRT_MT)
#MSVC_USE_PCH(${PROJECT_NAME} ${entry}/stdafx.h ${entry}/stdafx.cc)
endif()
