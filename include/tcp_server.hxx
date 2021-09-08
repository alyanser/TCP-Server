#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP
#pragma once

#include <shared_mutex>
#include <thread>
#include <set>
#include <map>
#include <atomic>
#include <random>
#include <asio/ip/tcp.hpp>
#include <asio/executor_work_guard.hpp>
#include <asio/ssl/context.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/io_context.hpp>
#include <asio/thread_pool.hpp>
#include <thread_safe_logger.hxx>

class tcp_server {
         using tcp_socket = asio::ip::tcp::socket;
         using ssl_tcp_socket = asio::ssl::stream<tcp_socket>;
public:
         enum { MINIMUM_THREAD_COUNT = 1, MAX_CONNECTIONS = 10, TIMEOUT_SECONDS = 5, MAX_ID = std::numeric_limits<uint64_t>::max() };

         tcp_server(uint8_t thread_count,uint16_t listen_port,std::string_view auth_dir);
         tcp_server(const tcp_server & rhs) = delete;
         tcp_server(tcp_server && rhs) = delete;
         tcp_server & operator = (const tcp_server & rhs) = delete;
         tcp_server & operator = (tcp_server && rhs) = delete;
         ~tcp_server();

         void start() noexcept;
         void shutdown() noexcept;
private: 
         [[nodiscard]] uint64_t get_spare_id() const noexcept;
         void listen() noexcept;
         void connection_timeout() noexcept;
         void configure_ssl_context() noexcept;
         void configure_acceptor() noexcept;
         void shutdown_socket(std::shared_ptr<ssl_tcp_socket> socket,const uint64_t client_id) noexcept;
         void attempt_handshake(std::shared_ptr<ssl_tcp_socket> ssl_socket,uint64_t client_id) noexcept;
         void read_message(std::shared_ptr<ssl_tcp_socket> ssl_socket,uint64_t client_id) noexcept;
         void respond(std::shared_ptr<ssl_tcp_socket> ssl_socket,std::string response,uint64_t client_id) noexcept;
         void process_message(std::shared_ptr<ssl_tcp_socket> ssl_socket,std::shared_ptr<std::string> request,uint64_t client_id,
                  const asio::error_code & connection_code) noexcept;
                  
///
         asio::io_context m_io_context;
         asio::ssl::context m_ssl_context = asio::ssl::context(asio::ssl::context::tlsv12_server);
         asio::executor_work_guard<asio::io_context::executor_type> m_executor_guard = asio::make_work_guard(m_io_context);
         asio::ip::tcp::acceptor m_acceptor = asio::ip::tcp::acceptor(m_io_context);
         std::atomic_bool m_server_running = false;
         std::atomic_uint32_t m_active_connections = 0;
         thread_safe_logger m_logger;
         std::set<uint64_t> m_active_client_ids;
         std::map<uint64_t,std::string> m_received_messages;
         mutable std::shared_mutex m_client_id_mutex;
         mutable std::shared_mutex m_received_messages_mutex;
         inline static std::mt19937 m_generator = std::mt19937(std::random_device()());
         inline static std::uniform_int_distribution m_id_range = std::uniform_int_distribution<uint64_t>(0,MAX_ID);
         
         uint16_t m_listen_port;
         std::string_view m_auth_dir;
         uint8_t m_thread_count;
         asio::thread_pool m_thread_pool;
};

inline tcp_server::~tcp_server(){
         shutdown();
}

inline void tcp_server::configure_ssl_context() noexcept {
         m_ssl_context.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::verify_peer);
         m_ssl_context.use_certificate_file(std::string(m_auth_dir) + "certificate.pem",asio::ssl::context_base::pem);
         m_ssl_context.use_rsa_private_key_file(std::string(m_auth_dir) + "private_key.pem",asio::ssl::context_base::pem);
}

inline void tcp_server::configure_acceptor() noexcept {
         asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(),m_listen_port);

         m_acceptor.open(endpoint.protocol());
         m_acceptor.set_option(asio::ip::tcp::socket::reuse_address(true));
         m_acceptor.bind(endpoint);
         m_logger.server_log("acceptor bound to port number",m_listen_port);
}

inline uint64_t tcp_server::get_spare_id() const noexcept {
         uint64_t unique_id;
         std::shared_lock client_id_guard(m_client_id_mutex);
         
         do{
                  unique_id = m_id_range(m_generator);
         }while(m_active_client_ids.count(unique_id));

         return unique_id;
}

#endif // TCP_SERVER_HPP