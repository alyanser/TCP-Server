cmake_minimum_required(VERSION 3.16.3)

project(tcpserver)

add_executable(${PROJECT_NAME} src/main.cxx)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_CXX_COMPILER "clang++-12")

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -fsanitize=address -fsanitize=undefined -Wall -Wshadow -Wold-style-cast -Wstrict-aliasing -Wdouble-promotion -Wundef -Wnull-dereference -Wfloat-equal -Wconversion -Woverflow -pedantic -Wextra -Wno-unused-parameter -Wfatal-errors")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fomit-frame-pointer -DNDEBUG")

add_definitions(-DASIO_STANDALONE)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${OpenSSL_LIBRARY}) 

include_directories("asio" "include" ${OpenSSL_INCLUDE_DIR})