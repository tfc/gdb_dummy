cmake_minimum_required (VERSION 3.10)
project (gdb_dummyserver)
set (CMAKE_CXX_STANDARD 17)

add_executable(gdb_dummyserver main.cpp)
target_include_directories(gdb_dummyserver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/attoparsecpp/include)
