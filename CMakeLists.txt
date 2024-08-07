cmake_minimum_required( VERSION 3.1 )

project(
    lib4zip
    VERSION 0.02
    LANGUAGES CXX)

option(AVX2 "Build with Avx2 optimisations" OFF)
    
include(CTest)

if (AVX2)
add_definitions(-mavx2)
add_definitions(-DAVX2)
message( "AVX2 enabled" )
else()
message( "AVX2 disabled" )
endif()

set(
    SOURCE_FILES
    aligned_string.h
    arith.h
    arith32.h
    arith64.h
    lib4zip.h
    bitio.h
    lzaahe_common.h
    lzaahe_context.h
    lzaahe_context.cpp
    lzaahe_dict.h
    lzaahe_dict.cpp
    lzaahe_encode.h
    lzaahe_encode.cpp
    lzaahe_decode.h
    lzaahe_decode.cpp
    los_context.h
    los_context.cpp
    los_decode.cpp
    los_decode.h
    los_encode.cpp
    los_encode.h
    platform.h)

add_library( 4zip STATIC ${SOURCE_FILES} )

add_subdirectory(sample)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)
endif()

enable_testing()

