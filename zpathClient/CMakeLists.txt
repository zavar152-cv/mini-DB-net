cmake_minimum_required(VERSION 3.23)
project(ZPathProject C)

set(CMAKE_C_STANDARD 17)

## Flex/Bison configuration
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(LibXml2 REQUIRED)

if (WIN32)
    set(ADDITIONAL_FLEX_FLAGS "--wincompat")
else ()
    set(ADDITIONAL_FLEX_FLAGS "")
endif ()
FLEX_TARGET(LEXER ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
        ${CMAKE_CURRENT_BINARY_DIR}/lexer.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/lexer.h COMPILE_FLAGS ${ADDITIONAL_FLEX_FLAGS})
BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
        ${CMAKE_CURRENT_BINARY_DIR}/parser.c COMPILE_FLAGS "-d -v")
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${LIBXML2_INCLUDE_DIR})

file(GLOB source_files
        ${CMAKE_CURRENT_SOURCE_DIR}/*.c
        )

add_executable(${PROJECT_NAME} ${source_files} ${BISON_PARSER_OUTPUTS} ${FLEX_LEXER_OUTPUTS} zgdbAst.h main.c zgdbAst.c zpathXml.c zpathXml.h)
target_link_libraries(${PROJECT_NAME} m ${LIBXML2_LIBRARIES})
