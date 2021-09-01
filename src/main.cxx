#include <iostream>
#include <functional>
#include <chrono>
#include <tcp_server.hpp>

int main(){
         enum { THREAD_COUNT = 4, LISTEN_PORT = 1234, SERVER_DURATION_SECONDS = 1000 };

         tcp_server server(THREAD_COUNT,LISTEN_PORT);
         std::this_thread::sleep_for(std::chrono::seconds(SERVER_DURATION_SECONDS));
}