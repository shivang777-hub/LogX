cmake_minimum_required(VERSION 3.10)
project(LogXSearch CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SEARCH_SOURCES
    src/LogSorter.cpp
    src/TimeSearch.cpp
    src/KeywordIndex.cpp
    src/TopKHeap.cpp
)

include_directories(include)

add_executable(main_test tests/main_test.cpp ${SEARCH_SOURCES})
