cmake_minimum_required(VERSION 3.5)
project(LogX_Trie LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(logx_trie STATIC Trie.cpp)
target_include_directories(logx_trie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_executable(TrieTest TrieTest.cpp)
target_link_libraries(TrieTest PRIVATE logx_trie)
add_test(NAME TrieTest COMMAND TrieTest)
