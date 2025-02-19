cmake_minimum_required(VERSION 3.10)
project(ZGDBProject C)

set(CMAKE_C_STANDARD 17)

find_package(LibXml2 REQUIRED)

include_directories("zgdb" "lib" "xml" ${LIBXML2_INCLUDE_DIR})

set(GENERAL_SRC zgdb/format/zgdbfile.c zgdb/format/zgdbfile.h zgdb/index/zgdbindex.c zgdb/index/zgdbindex.h
        zgdb/data/zgdbdata.c zgdb/data/zgdbdata.h zgdb/index/list/freelist.c zgdb/index/list/freelist.h
        zgdb/schema/zgdbschema.c zgdb/schema/zgdbschema.h zgdb/zgdb.c zgdb/zgdb.h zgdb/data/result/resultlist.c zgdb/data/result/resultlist.h
        zgdb/data/treestack/treestack.c zgdb/data/treestack/treestack.h zgdb/data/iterator/eliterator.c zgdb/data/iterator/eliterator.h
        zgdb/data/iterator/dociterator.c zgdb/data/iterator/dociterator.h zgdb/data/filter/zgdbfilter.c zgdb/data/filter/zgdbfilter.h
        zgdb/data/result/elresultlist.c zgdb/data/result/elresultlist.h xml/zgdbXmlRequest.c xml/zgdbXmlRequest.h)

set(MAIN main.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mno-ms-bitfields")
if (WIN32)
    set(WIN_SRC lib/mman-win32/mman.c lib/mman-win32/mman.h lib/fmemopen_windows/libfmemopen.c lib/fmemopen_windows/libfmemopen.h xml/zgdbXmlRequest.c xml/zgdbXmlRequest.h)
    add_executable(ZGDBProject ${WIN_SRC} ${GENERAL_SRC} ${MAIN})
endif (WIN32)

if (UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address")
    add_executable(${PROJECT_NAME} ${GENERAL_SRC} ${MAIN})
    target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})
endif (UNIX)

