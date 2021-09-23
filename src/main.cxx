#include "tcp_server.hxx"

#include <chrono>
#include <string>

int main(){
         constexpr auto thread_count = 3;
         constexpr auto listen_port = 1234;
         constexpr std::string_view auth_dir("../certs/");

         Tcp_server server(thread_count,listen_port,auth_dir);
         server.start();
         
         // emulate
         constexpr auto server_duration_seconds = 1000;
         std::this_thread::sleep_for(std::chrono::seconds(server_duration_seconds));
}