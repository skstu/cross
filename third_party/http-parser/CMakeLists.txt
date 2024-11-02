#project (${entry_name})
project(http_parser)
#GFN_PROJECT_SOURCE_GET(${CMAKE_CURRENT_SOURCE_DIR}/src OUT_HEADER_S OUT_SOURCE_S)
set(SOURCES 
http_parser.c
http_parser_method.cxx
)


if(MSVC)
    add_compile_options(/wd4996 /wd4267)
endif()


add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} 
PRIVATE
${SOLUTION_INCLUDE_DIR}
${SOLUTION_INCLUDE_DIR}/3rdlocal/http-parser/
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/include
)

#if(MSVC)
#MSVC_LINK_FLAGS_SET_CONSOLE(${PROJECT_NAME})
#MSVC_CRT_SET(${PROJECT_NAME} "/MT")
#MSVC_USE_PCH(${PROJECT_NAME} ${entry}/stdafx.h ${entry}/stdafx.cc)
#endif()

#[[
set(TARGET_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${parent_dir_name}/${PROJECT_NAME}/${CMAKE_SYSTEM_NAME}")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${TARGET_INSTALL_DIR}/${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${TARGET_INSTALL_DIR}/bin/
    LIBRARY DESTINATION ${TARGET_INSTALL_DIR}/lib/
    ARCHIVE DESTINATION ${TARGET_INSTALL_DIR}/lib/
)
]]