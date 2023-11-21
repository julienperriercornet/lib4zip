
project(testlib4zip LANGUAGES CXX)

add_executable(testlib4zip test.cpp)

target_link_libraries(testlib4zip PRIVATE 4zip)

add_test(NAME test_lzaahe_context COMMAND testlib4zip test_lzaahe_context)
add_test(NAME test_lzaahe_sort COMMAND testlib4zip test_lzaahe_sort)
