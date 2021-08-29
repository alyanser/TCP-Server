#include <iostream>
#include <algorithm>
#include <memory>
#include <random>
#include <future>
#include <asio/ssl.hpp>
#include <tcp_server.hpp>

tcp_server::tcp_server(uint8_t thread_count,const uint16_t listen_port)
         : m_ssl_context(asio::ssl::context::tlsv12_server), m_resolver(m_io_context), m_executor_guard(m_io_context.get_executor()), m_listen_port(listen_port), logger(m_mutex)
{
         auto worker_thread = [&m_io_context = m_io_context,&m_mutex = m_mutex](){
                  while(true){
                           try{
                                    m_io_context.run();
                                    break; // exit if thread gracefully ended
                           }catch(const std::exception & exception){
                                    std::lock_guard print_guard(m_mutex);
                           }
                  }
         };
                  
         thread_count = std::max(thread_count,static_cast<uint8_t>(MINIMUM_THREAD_COUNT));

         m_thread_pool.reserve(thread_count);

         for(uint8_t i = 0;i < thread_count;i++){
                  m_thread_pool.emplace_back(std::thread(worker_thread));
         }

         m_io_context.post(std::bind(&tcp_server::listen,this));
}

tcp_server::~tcp_server(){
         m_executor_guard.reset(); // inform the threads there is no more work to do
         for(auto & thread : m_thread_pool) thread.join();
}

void tcp_server::listen(){
         asio::ip::tcp::acceptor acceptor(m_io_context);

         {        // configure acceptor
                  asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::any(),m_listen_port);
                  acceptor.open(endpoint.protocol());
                  acceptor.bind(endpoint);
                  acceptor.listen();
         }

         while(true){
                  try{
                           // stream with an underlying tcp socket used for tls
                           auto ssl_stream = std::make_shared<asio::ssl::stream<tcp_socket>>(m_io_context,m_ssl_context);
                           // get a spare id asyncronously
                           using task_type = std::packaged_task<uint64_t()>;
                           auto client_id_task = std::make_shared<task_type>(std::bind(&tcp_server::get_spare_id,this));
                           auto client_id_future = client_id_task->get_future();
                           m_io_context.post(std::bind(&task_type::operator(),client_id_task));
                           // wait for a client to connect
                           acceptor.accept(ssl_stream->lowest_layer()); // lowest layer being the socket
                           // get the id
                           auto new_client_id = client_id_future.get();
                           active_client_ids.insert(new_client_id);
                           // forward client to different thread and wait for next client
                           m_io_context.post(std::bind(&tcp_server::handle_client,this,ssl_stream,new_client_id));
                  }catch(const std::exception & exception){
                           logger.error_log(exception.what(),'\n');
                  }
         }
}

void tcp_server::handle_client(std::shared_ptr<asio::ssl::stream<tcp_socket>> ssl_stream,const uint64_t client_id){
         logger.server_log("New client connected. Id : ",client_id,".\n");
         ssl_stream->handshake(asio::ssl::stream_base::handshake_type::server);
}

uint64_t tcp_server::get_spare_id() const noexcept {
         static std::mt19937 generator(std::random_device{}());
         static std::uniform_int_distribution<uint64_t> id_range(0,std::numeric_limits<uint64_t>::max());

         uint64_t unique_id;

         do{
                  unique_id = id_range(generator);
         }while(active_client_ids.count(unique_id));

         return unique_id;
}