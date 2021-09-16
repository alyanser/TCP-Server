#ifndef SERVER_LOGGER_HXX
#define SERVER_LOGGER_HXX

#include <iostream>
#include <mutex>

class thread_safe_logger {
public:
         template<typename ... Args>
         void server_log(Args && ... args) const noexcept;

         template<typename ... Args>
         void error_log(Args && ... args) const noexcept;
         
         template<typename Message>
         void receive_log(uint64_t client_id,Message && message) const noexcept;

         template<typename Message>
         void send_log(uint64_t client_id,Message && message) const noexcept;

private:
         mutable std::mutex m_print_mutex;
};

template<typename ... Args>
inline void thread_safe_logger::server_log(Args && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "[server] : ";
         ((std::cout << args << ' '),...) << '\n';
}

template<typename ... Args>
inline void thread_safe_logger::error_log(Args && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cerr << "[error_] : ";
         ((std::cerr << args << ' '),...) << '\n';
}

template<typename Message>
inline void thread_safe_logger::receive_log(const uint64_t client_id,Message && message) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "\n*** message from client [" << client_id << "]\n\n\t--- START MESSAGE ---\n" << message
                            << "\n\t--- END MESSAGE" "---\n\n";
}

template<typename Message>
inline void thread_safe_logger::send_log(const uint64_t client_id,Message && message) const noexcept {
         std::lock_guard guard(m_print_mutex);
         
         std::cout << "\n*** response sent to client [" << client_id << "]\n\n\t--- START RESPONSE ---\n" << message
                            << "\n\t--- END" "RESPONSE---\n\n";
}

#endif // SERVER_LOGGER_HXX
