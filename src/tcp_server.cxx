#include <iostream>
#include <tcp_server.hpp>
#include <asio/ssl.hpp>
#include <memory>

tcp_server::tcp_server(const uint8_t thread_count,const uint16_t listen_port)
         : m_ssl_context(asio::ssl::context::tlsv12_server), m_resolver(m_io_context), m_executor_guard(m_io_context.get_executor())
         , m_listen_port(listen_port)
{
         auto worker_thread = [&m_io_context = m_io_context,&m_mutex = m_mutex](){
                  while(true){
                           try{
                                    m_io_context.run();
                                    break; // exit if thread gracefully ended
                           }catch(const std::exception & exception){
                                    std::lock_guard print_guard(m_mutex);
                                    std::cerr << exception.what() << '\n';
                           }
                  }
         };

                  
         m_thread_pool.reserve(thread_count);

         for(uint8_t i = 0;i < thread_count;i++){
                  m_thread_pool.emplace_back(std::thread(worker_thread));
         }

         //TODO choose if to make it blocking or non blocking
         listen();
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
                  auto ssl_stream = std::make_shared<asio::ssl::stream<tcp_socket>>(m_io_context,m_ssl_context);
                  
                  acceptor.accept(ssl_stream->lowest_layer());

                  m_io_context.post(std::bind(&tcp_server::handle_client,this,ssl_stream));
         }
}

void tcp_server::handle_client(std::shared_ptr<asio::ssl::stream<tcp_socket>> ssl_stream){
         std::lock_guard guard(m_mutex);
         std::cout << "Clisent connected\n";
}