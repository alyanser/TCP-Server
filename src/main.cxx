#include <chrono>
#include <string>
#include <tcp_server.hpp>

int main(){
         enum { THREAD_COUNT = 4, LISTEN_PORT = 1234, SERVER_DURATION_SECONDS = 10000 };

         std::string auth_dir("../certs");
         tcp_server server(THREAD_COUNT,LISTEN_PORT,auth_dir);
         server.start();
         std::this_thread::sleep_for(std::chrono::seconds(SERVER_DURATION_SECONDS));
}