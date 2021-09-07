cmake_minimum_required(VERSION 3.16.3)

project(tcpserver)

set(CMAKE_CXX_COMPILER "clang++-12")
set(CMAKE_CXX_STANDARD "20")

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -fsanitize=address -fsanitize=undefined -Wall -Wshadow -Wold-style-cast -Wstrict-aliasing -Wdouble-promotion -Wundef -Wnull-dereference -Wfloat-equal -Wconversion -Woverflow -pedantic -Wextra -Wfatal-errors")

add_definitions(-DASIO_STANDALONE)

set(SOURCES
         src/main.cxx
         src/tcp_server.cxx
)

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(
         SYSTEM "asio"
         "include"
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} 
         Threads::Threads
         ${OPENSSL_LIBRARIES}
)