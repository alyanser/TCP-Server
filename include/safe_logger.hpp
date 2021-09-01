#ifndef LOGGER_HPP
#define LOGGER_HPP
#pragma once

#include <iostream>
#include <mutex>

class safe_logger {
public:
         safe_logger(std::mutex & mutex) : m_mutex(mutex){}

         template<typename ... Args>
         void server_log(Args && ... args) const noexcept {
                  std::lock_guard guard(m_mutex);
                  std::cout << "[server] : ";
                  ((std::cout << args),...) << '\n';
         }

         template<typename ... Args>
         void client_log(Args && ... args) const noexcept {
                  std::lock_guard guard(m_mutex);
                  std::cout << "[client] : ";
                  ((std::cout << args),...) << '\n';
         }

         template<typename ... Args>
         void error_log(Args && ... args) const noexcept {
                  std::lock_guard guard(m_mutex);
                  std::cerr << "[_error] : ";
                  ((std::cerr << args),...) << '\n';
         }

private:
         std::mutex & m_mutex;
};

#endif