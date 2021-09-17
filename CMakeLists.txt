cmake_minimum_required(VERSION 3.16.3)

project(tcpserver)

set(CMAKE_CXX_COMPILER "clang++-13")
set(CMAKE_CXX_STANDARD "17")
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CXX_FLAGS "-pipe -pedantic -Wall -Wextra -Wshadow -Wdouble-promotion -Wcast-align -Wconversion -Wfloat-equal -Wunreachable-code")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -fsanitize=undefined,address -fno-omit-frame-pointer ${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${CXX_FLAGS}")

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