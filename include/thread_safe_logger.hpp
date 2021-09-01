#ifndef THREAD_SAFE_LOGGER_HPP
#define THREAD_SAFE_LOGGER_HPP
#pragma once

#include <iostream>
#include <mutex>

class thread_safe_logger {
public:
         template<typename ... Args>
         void server_log(Args && ... args) const noexcept {
                  std::lock_guard guard(p_print_mutex);
                  std::cout << "[server] : ";
                  ((std::cout << args << ' '),...) << '\n';
         }

         void receive_log(const uint64_t client_id,const std::string & request) const noexcept {
                  std::lock_guard guard(p_print_mutex);
                  std::cout << "request from client [" << client_id << "]\n\t--- START REQUEST\n" << request << "\n\t--- END REQUEST ---\n";
         }

         void send_log(const uint64_t client_id,const std::string & request) const noexcept {
                  std::lock_guard guard(p_print_mutex);
                  std::cout << "response sent to client [" << client_id << "]\n\t--- START RESPONSE\n" << request << "\n\t--- END RESPONSE ---\n";
         }

         template<typename ... Args>
         void error_log(Args && ... args) const noexcept {
                  std::lock_guard guard(p_print_mutex);
                  std::cerr << "[error_] : ";
                  ((std::cerr << args << ' '),...) << '\n';
         }
private:
         mutable std::mutex p_print_mutex;
};

#endif // SAFE_LOGGER_HPP