#include <iostream>
#include <algorithm>
#include <memory>
#include <random>
#include <future>
#include <asio/ssl.hpp>
#include <tcp_server.hpp>
#include <asio/dispatch.hpp>
#include <asio/steady_timer.hpp>
#include <asio/placeholders.hpp>

tcp_server::tcp_server(uint8_t thread_count,const uint16_t listen_port)
         : m_ssl_context(asio::ssl::context::sslv23_server), m_resolver(m_io_context), m_executor_guard(m_io_context.get_executor())
         , m_logger(m_mutex), m_acceptor(m_io_context), m_listen_port(listen_port)
{
         auto worker_thread = [this](){
                  while(m_server_running){
                           try{
                                    m_io_context.run();
                                    break; // stop if thread gracefully ended
                           }catch(const std::exception & exception){
                                    m_logger.error_log(exception.what());
                           }
                  }
         };

         thread_count = std::max(thread_count,static_cast<uint8_t>(MINIMUM_THREAD_COUNT));

         m_thread_pool.reserve(thread_count);

         for(uint8_t i = 0;i < thread_count;i++){
                  m_thread_pool.emplace_back(std::thread(worker_thread));
         }

         m_logger.server_log("started with ",static_cast<uint16_t>(thread_count)," threads");

         configure_acceptor();
         asio::post(m_io_context,std::bind(&tcp_server::listen,this));
}

tcp_server::~tcp_server(){
         shutdown();
}

void tcp_server::shutdown() noexcept {
         m_server_running = false;

         m_executor_guard.reset(); // inform the threads there is no more work to do
         m_acceptor.cancel(); // stop any on-going operations
         m_io_context.stop();

         m_logger.server_log("status changed to non-listening state");

         for(auto & thread : m_thread_pool){
                  assert(thread.joinable());
                  thread.join();
         }

         m_logger.server_log("shutdown");
}

void tcp_server::configure_acceptor(){
         asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(),m_listen_port);
         m_acceptor.open(endpoint.protocol());
         m_acceptor.bind(endpoint);
         m_logger.server_log("acceptor bound to port number ",m_listen_port);
}

void tcp_server::connection_timeout(){
         m_acceptor.cancel();
         m_logger.server_log("status changed to non-listening state");

         auto timeout_timer = std::make_shared<asio::steady_timer>(m_io_context);

         timeout_timer->expires_from_now(std::chrono::seconds(TIMEOUT_SECONDS));

         timeout_timer->async_wait([this,timeout_timer](const asio::error_code & ec) noexcept {
                  if(!ec){
                           m_logger.server_log("connection timeout over. shifting to listening state");
                           asio::post(m_io_context,std::bind(&tcp_server::listen,this));
                  }else{
                           m_logger.error_log(ec);
                  }
         });
}

void tcp_server::listen(){
         m_acceptor.listen();
         m_logger.server_log("status changed to listening state");

         if(m_active_connections == MAX_CONNECTIONS){
                  m_logger.error_log("max connections reached. taking a connection timeout for ",TIMEOUT_SECONDS," seconds");
                  asio::post(m_io_context,std::bind(&tcp_server::connection_timeout,this));
                  return;
         }

         auto ssl_socket = std::make_shared<ssl_tcp_socket>(m_io_context,m_ssl_context);

         // get a spare id asyncronously
         using task_type = std::packaged_task<uint64_t()>;
         auto client_id_task = std::make_shared<task_type>(std::bind(&tcp_server::get_spare_id,this));
         asio::post(m_io_context,std::bind(&task_type::operator(),client_id_task));

         auto on_accept = [this,ssl_socket,client_id_task](const asio::error_code & error_code) noexcept {
                  if(!error_code){
                           // client connected. get the id
                           auto new_client_id = client_id_task->get_future().get();
                           m_active_client_ids.insert(new_client_id);
                           m_active_connections++;
                           assert(m_active_connections == m_active_client_ids.size());

                           m_logger.server_log("client [",new_client_id,"] attempting to connect. proceeding to handshake");
                           // forward client to different thread and wait for next client
                           asio::post(m_io_context,std::bind(&tcp_server::handle_client,this,ssl_socket,new_client_id));
                           // post for the next client
                           asio::post(m_io_context,std::bind(&tcp_server::listen,this));
                  }else{
                           m_logger.error_log(error_code);
                  }
         };

         m_logger.server_log("waiting for client to connect");
         // asyncronously wait for a client to connect
         m_acceptor.async_accept(ssl_socket->lowest_layer(),std::bind(on_accept,std::placeholders::_1));
}

void tcp_server::handle_client(std::shared_ptr<ssl_tcp_socket> ssl_socket,const uint64_t client_id){
         m_logger.server_log("handshake attempt with client [",client_id,']');
         ssl_socket->handshake(asio::ssl::stream_base::handshake_type::server);
         m_logger.server_log("handshake successful. connected with client [",client_id,']');

         ssl_socket->lowest_layer().cancel(); // cancel any pending socket oeprations
         ssl_socket->lowest_layer().shutdown(tcp_socket::shutdown_both); // close read and write descriptors
}

uint64_t tcp_server::get_spare_id() const noexcept {
         static std::mt19937 generator(std::random_device{}());
         static std::uniform_int_distribution<uint64_t> id_range(0,std::numeric_limits<uint64_t>::max());

         uint64_t unique_id;

         do{
                  unique_id = id_range(generator);
         }while(m_active_client_ids.count(unique_id));

         return unique_id;
}