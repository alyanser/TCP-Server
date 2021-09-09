#include <chrono>
#include <string>
#include <tcp_server.hxx>

int main(){
         constexpr uint8_t thread_count = 1;
         constexpr uint16_t listen_port = 3;
         constexpr std::string_view auth_dir("../certs/");

         tcp_server server(thread_count,listen_port,auth_dir);

         server.start();
         // emulate
         constexpr uint64_t server_duration_seconds = 3;
         std::this_thread::sleep_for(std::chrono::seconds(server_duration_seconds));
}