#ifndef SERVER_LOGGER_HXX
#define SERVER_LOGGER_HXX

#include <iostream>
#include <mutex>

class Server_logger {
public:
         template<typename ... Args_T>
         void server_log(Args_T && ... args) const noexcept;

         template<typename ... Args_T>
         void error_log(Args_T && ... args) const noexcept;
         
         template<typename Message_T>
         void receive_log(std::uint64_t client_id,Message_T && message) const noexcept;

         template<typename Message_T>
         void send_log(std::uint64_t client_id,Message_T && message) const noexcept;

private:
         mutable std::mutex m_print_mutex;
};

template<typename ... Args>
void Server_logger::server_log(Args && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "[server] : ";
         ((std::cout << args << ' '),...) << '\n';
}

template<typename ... Args>
void Server_logger::error_log(Args && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cerr << "[error_] : ";
         ((std::cerr << args << ' '),...) << '\n';
}

template<typename Message>
void Server_logger::receive_log(const std::uint64_t client_id,Message && message) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "\n*** message from client [" << client_id << "]\n\n\t--- START MESSAGE ---\n" << message
                            << "\n\t--- END MESSAGE" "---\n\n";
}

template<typename Message>
void Server_logger::send_log(const std::uint64_t client_id,Message && message) const noexcept {
         std::lock_guard guard(m_print_mutex);
         
         std::cout << "\n*** response sent to client [" << client_id << "]\n\n\t--- START RESPONSE ---\n" << message
                            << "\n\t--- END" "RESPONSE---\n\n";
}

#endif // SERVER_LOGGER_HXX