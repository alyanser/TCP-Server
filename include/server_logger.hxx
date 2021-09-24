#ifndef SERVER_LOGGER_HXX
#define SERVER_LOGGER_HXX

#include <iostream>
#include <mutex>

class Server_logger {
public:
         template<typename ... args_type>
         void server_log(args_type && ... args) const noexcept;

         template<typename ... args_type>
         void error_log(args_type && ... args) const noexcept;
         
         template<typename message_type>
         void receive_log(std::uint64_t client_id,message_type && message) const noexcept;

         template<typename message_type>
         void send_log(std::uint64_t client_id,message_type && message) const noexcept;

private:
         mutable std::mutex m_print_mutex;
};

template<typename ... args_type>
void Server_logger::server_log(args_type && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "[server] : ";
         ((std::cout << args << ' '),...) << '\n';
}

template<typename ... args_type>
void Server_logger::error_log(args_type && ... args) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cerr << "[error_] : ";
         ((std::cerr << args << ' '),...) << '\n';
}

template<typename message_type>
void Server_logger::receive_log(const std::uint64_t client_id,message_type && message) const noexcept {
         std::lock_guard guard(m_print_mutex);

         std::cout << "\n*** message from client [" << client_id << "]\n\n\t--- START MESSAGE ---\n" << message
                            << "\n\t--- END MESSAGE" "---\n\n";
}

template<typename message_type>
void Server_logger::send_log(const std::uint64_t client_id,message_type && message) const noexcept {
         std::lock_guard guard(m_print_mutex);
         
         std::cout << "\n*** response sent to client [" << client_id << "]\n\n\t--- START RESPONSE ---\n" << message
                            << "\n\t--- END" "RESPONSE---\n\n";
}

#endif // SERVER_LOGGER_HXX