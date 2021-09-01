cmake_minimum_required(VERSION 3.16.3)

project(tcpserver)

set(CMAKE_CXX_COMPILER "g++-11")
set(CMAKE_CXX_STANDARD "20")
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(DEBUG_FLAGS "-ggdb3 -Wall -Wshadow -Wold-style-cast -Wstrict-aliasing -Wdouble-promotion -Wundef -Wnull-dereference -Wfloat-equal -Wconversion -Woverflow -pedantic -Wextra -Wfatal-errors")

set(DEBUG_FLAGS_WITH_SANITIZER "-fsanitize=address -fsanitize=undefined -ggdb3 -Wall -Wshadow -Wold-style-cast -Wstrict-aliasing -Wdouble-promotion -Wundef -Wnull-dereference -Wfloat-equal -Wconversion -Woverflow -pedantic -Wextra -Wfatal-errors")

set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS_WITH_SANITIZER})

add_definitions(-DASIO_STANDALONE)

add_executable(${PROJECT_NAME} 
         src/main.cxx 
         src/tcp_server.cxx
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} 
         Threads::Threads
         ${OPENSSL_LIBRARIES}
)

include_directories(
         SYSTEM "asio"
         "include"
)