cmake_minimum_required(VERSION 3.16.3)

project(tcpserver CXX)

set(CMAKE_CXX_STANDARD "17")
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_definitions(-DASIO_STANDALONE)

set(SOURCES
         src/main.cc
         src/tcp_server.cc
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
         "ext/asio"
)

target_include_directories(${PROJECT_NAME} PRIVATE
         "include"
)

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} 
         Threads::Threads
         ${OPENSSL_LIBRARIES}
)