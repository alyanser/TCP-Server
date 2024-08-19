#ifndef TCP_SERVER_HXX
#define TCP_SERVER_HXX

#include "server_logger.h"

#include <asio/executor_work_guard.hpp>
#include <asio/thread_pool.hpp>
#include <asio/ssl/context.hpp>
#include <asio/ssl/stream.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <random>
#include <set>
#include <map>

class Tcp_server {
public:
	using tcp_socket = asio::ip::tcp::socket;
	using ssl_tcp_socket = asio::ssl::stream<tcp_socket>;

	struct Network_message {
		std::shared_ptr<ssl_tcp_socket> ssl_socket;
		std::shared_ptr<std::string> content;
		std::uint64_t client_id;
	};

	Tcp_server(std::uint8_t thread_count, std::uint16_t listen_port, std::string_view auth_dir);
	Tcp_server(const Tcp_server & rhs) = delete;
	Tcp_server(Tcp_server && rhs) = delete;
	Tcp_server & operator=(const Tcp_server & rhs) = delete;
	Tcp_server & operator=(Tcp_server && rhs) = delete;
	~Tcp_server();

	void start() noexcept;
	void shutdown() noexcept;

private:
	std::uint64_t get_random_spare_id() const noexcept;
	void listen() noexcept;
	void connection_timeout() noexcept;
	void configure_ssl_context() noexcept;
	void configure_acceptor() noexcept;
	void shutdown_socket(std::shared_ptr<ssl_tcp_socket> socket, std::uint64_t client_id) noexcept;
	void attempt_handshake(std::shared_ptr<ssl_tcp_socket> ssl_socket, std::uint64_t client_id) noexcept;
	void read_message(std::shared_ptr<ssl_tcp_socket> ssl_socket, std::uint64_t client_id) noexcept;
	void respond(std::shared_ptr<ssl_tcp_socket> ssl_socket, std::string response, std::uint64_t client_id) noexcept;
	void process_message(const Network_message & message, const asio::error_code & connection_code) noexcept;
	///
	constexpr static auto minimum_thread_count = 1;
	constexpr static auto max_connections = 100;
	constexpr static auto timeout_seconds = 5;
	inline static std::mt19937 random_generator{std::random_device()()};
	inline static std::uniform_int_distribution<std::uint64_t> random_id_range;

	asio::io_context m_io_context;
	asio::ssl::context m_ssl_context{asio::ssl::context::tlsv12_server};
	asio::executor_work_guard<asio::io_context::executor_type> m_executor_guard = asio::make_work_guard(m_io_context);
	asio::ip::tcp::acceptor m_acceptor{m_io_context};
	std::set<std::uint64_t> m_active_client_ids;
	std::map<std::uint64_t, std::string> m_received_messages;
	std::atomic_bool m_server_running = false;
	std::atomic_uint32_t m_active_connections = 0;
	Server_logger m_logger;
	mutable std::shared_mutex m_client_id_mutex;
	mutable std::shared_mutex m_received_messages_mutex;

	std::uint16_t m_listen_port = 0;
	std::string_view m_auth_dir;
	std::uint8_t m_thread_count = 0;
	asio::thread_pool m_thread_pool;
};

inline Tcp_server::Tcp_server(const std::uint8_t thread_count, const std::uint16_t listen_port, const std::string_view auth_dir)
    : m_listen_port(listen_port), m_auth_dir(auth_dir), m_thread_count(std::max<std::uint8_t>(thread_count, minimum_thread_count)),
	m_thread_pool(m_thread_count) {
}

inline Tcp_server::~Tcp_server() {
	shutdown();
}

#endif // TCP_SERVER_HXX