#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP
#pragma once

#include <vector>
#include <shared_mutex>
#include <thread>
#include <set>
#include <map>
#include <atomic>
#include <thread_safe_logger.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <asio/executor_work_guard.hpp>

class tcp_server {
         using tcp_socket = asio::ip::tcp::socket;
         using ssl_tcp_socket = asio::ssl::stream<tcp_socket>;
public:
         enum { MINIMUM_THREAD_COUNT = 1, MAX_CONNECTIONS = 10, TIMEOUT_SECONDS = 5 };

         tcp_server(uint8_t thread_count,uint16_t listen_port,const std::string & auth_dir);
         tcp_server(const tcp_server & rhs) = delete;
         tcp_server(tcp_server && rhs) = delete;
         tcp_server & operator = (const tcp_server & rhs) = delete;
         tcp_server & operator = (tcp_server && rhs) = delete;
         ~tcp_server();

         void start() noexcept;
         void shutdown() noexcept;
private: 
         void listen() noexcept;
         void connection_timeout() noexcept;
         void configure_ssl_context() noexcept;
         void configure_acceptor() noexcept;
         uint64_t get_spare_id() const noexcept;
         void shutdown_socket(std::shared_ptr<ssl_tcp_socket> socket,const uint64_t client_id) noexcept;
         void attempt_handshake(std::shared_ptr<ssl_tcp_socket> ssl_socket,uint64_t client_id) noexcept;
         void read_message(std::shared_ptr<ssl_tcp_socket> ssl_socket,uint64_t client_id) noexcept;
         void respond(std::shared_ptr<ssl_tcp_socket> ssl_socket,std::string response,uint64_t client_id) noexcept;
         void process_message(std::shared_ptr<ssl_tcp_socket> ssl_socket,std::shared_ptr<std::string> request,uint64_t client_id,
                  const asio::error_code & connection_code) noexcept;
private:
         asio::io_context m_io_context;
         asio::ssl::context m_ssl_context;
         asio::executor_work_guard<asio::io_context::executor_type> m_executor_guard;
         asio::ip::tcp::acceptor m_acceptor;
         const uint16_t m_listen_port;
         std::string m_auth_dir;
         const uint8_t m_thread_count;
         thread_safe_logger m_logger;

         std::atomic_bool m_server_running = false;
         std::atomic_uint32_t m_active_connections = 0;
         std::vector<std::thread> m_thread_pool;
         std::set<uint64_t> m_active_client_ids;
         std::map<uint64_t,std::string> m_received_messages;
         mutable std::shared_mutex m_client_id_mutex;
         mutable std::shared_mutex m_received_messages_mutex;
};

#endif // TCP_SERVER_HPP