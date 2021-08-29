#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP
#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <set>
#include <atomic>
#include <safe_logger.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/ssl.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <dns_resolver.hpp>
#include <asio/executor_work_guard.hpp>

class tcp_server {
         enum { MINIMUM_THREAD_COUNT = 1, MAX_CONNECTIONS = 100, TIMEOUT_SECONDS = 5 };
         using tcp_socket = asio::ip::tcp::socket;
         using ssl_tcp_socket = asio::ssl::stream<tcp_socket>;
public:
         tcp_server(uint8_t thread_count,uint16_t listen_port);
         tcp_server(const tcp_server & rhs) = delete;
         tcp_server(tcp_server && rhs) = delete;
         ~tcp_server();
private: 
         void shutdown() noexcept;
         void connection_timeout();
         void configure_acceptor();
         void listen();
         void handle_client(std::shared_ptr<ssl_tcp_socket> ssl_stream,uint64_t client_id);
         uint64_t get_spare_id() const noexcept;
private:
         asio::io_context m_io_context;
         asio::ssl::context m_ssl_context;
         asio::executor_work_guard<asio::io_context::executor_type> m_executor_guard;
         dns_resolver m_resolver;
         safe_logger m_logger;
         asio::ip::tcp::acceptor m_acceptor;
         const uint16_t m_listen_port;
         std::mutex m_mutex;
         std::atomic<bool> m_server_running = true;
         std::vector<std::thread> m_thread_pool;
         std::set<uint64_t> m_active_client_ids;
         uint32_t m_active_connections = 0;
};

#endif