#ifndef THREAD_SAFE_LOGGER_HPP
#define THREAD_SAFE_LOGGER_HPP
#pragma once

#include <iostream>
#include <mutex>

class thread_safe_logger {
public:
         template<typename ... Args>
         void server_log(Args && ... args) const noexcept {
                  std::lock_guard guard(m_print_mutex);
                  std::cout << "[server] : ";
                  ((std::cout << args << ' '),...) << '\n';
         }

         template<typename ... Args>
         void error_log(Args && ... args) const noexcept {
                  std::lock_guard guard(m_print_mutex);
                  std::cerr << "[error_] : ";
                  ((std::cerr << args << ' '),...) << '\n';
         }
         
         template<typename Message>
         void receive_log(const uint64_t client_id,Message && message) const noexcept {
                  std::lock_guard guard(m_print_mutex);
                  std::cout << "\n*** message from client [" << client_id << "]\n\n\t--- START MESSAGE ---\n" << message << "\n\t--- END MESSAGE" "---\n\n";
         }

         template<typename Message>
         void send_log(const uint64_t client_id,Message && message) const noexcept {
                  std::lock_guard guard(m_print_mutex);
                  std::cout << "\n*** response sent to client [" << client_id << "]\n\n\t--- START RESPONSE ---\n" << message << "\n\t--- END" "RESPONSE---\n\n";
         }

private:
         mutable std::mutex m_print_mutex;
};

#endif // SAFE_LOGGER_HPP