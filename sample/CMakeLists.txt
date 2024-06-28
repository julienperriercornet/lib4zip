project(samplelib4zip LANGUAGES CXX)

add_executable(samplelib4zip main.cpp)

target_link_libraries(samplelib4zip PRIVATE 4zip)

add_executable(turbosqueeze turbosqueeze.cpp)

target_link_libraries(turbosqueeze PRIVATE 4zip)
