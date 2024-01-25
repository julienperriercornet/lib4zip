
project(testlib4zip LANGUAGES CXX)

add_executable(testlib4zip test_arith32.cpp test_arith64.cpp test.cpp)

target_link_libraries(testlib4zip PRIVATE 4zip)

add_test(NAME test_arith32 COMMAND testlib4zip test_arith32)
add_test(NAME test_arith64 COMMAND testlib4zip test_arith64)
add_test(NAME test_lzaahe_context COMMAND testlib4zip test_lzaahe_context)
add_test(NAME test_lzaahe_sort COMMAND testlib4zip test_lzaahe_sort)
add_test(NAME test_bitio COMMAND testlib4zip test_bitio)
