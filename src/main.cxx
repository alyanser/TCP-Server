#include <chrono>
#include <string>
#include <tcp_server.hxx>

int main(){
         enum { THREAD_COUNT = 3, LISTEN_PORT = 1234, SERVER_DURATION_SECONDS = 10000 };

         constexpr std::string_view auth_dir("../certs/");
         tcp_server server(THREAD_COUNT,LISTEN_PORT,auth_dir);

         server.start();
         // emulate
         std::this_thread::sleep_for(std::chrono::seconds(SERVER_DURATION_SECONDS));
}