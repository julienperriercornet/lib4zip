cmake_minimum_required( VERSION 3.1 )

project(
    lib4zip
    VERSION 0.01
    LANGUAGES CXX)

include(CTest)

set(
    SOURCE_FILES
    lib4zip.h
    arith.h
    arith32.h
    arith64.h
    bitio.h
    lzaahe_common.h
    lzaahe_context.h
    lzaahe_context.cpp
    lzaahe_dict.h
    lzaahe_dict.cpp
    lzaahe_encode.h
    lzaahe_encode.cpp
    platform.h)

add_library( 4zip STATIC ${SOURCE_FILES} )

add_subdirectory(sample)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)
endif()

enable_testing()

