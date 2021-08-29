#include <iostream>
#include <chrono>
#include <tcp_server.hpp>

int main(){
         tcp_server server(4,1234);
         
         std::this_thread::sleep_for(std::chrono::hours(1));
}