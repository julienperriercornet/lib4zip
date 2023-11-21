cmake_minimum_required( VERSION 3.1 )

project(
    lib4zip
    VERSION 0.01
    LANGUAGES CXX)

find_package(Catch2 CONFIG REQUIRED)

include(CTest)

set(
    SOURCE_FILES
    arith.h
    arith32.h
    arith64.h
    lzaahe_common.h
    lzaahe_context.h
    lzaahe_context.cpp)

add_library( 4zip STATIC ${SOURCE_FILES} )

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)
endif()

enable_testing()

