#include "tcp_server.hxx"

#include <asio/steady_timer.hpp>
#include <asio/error.hpp>
#include <asio/dispatch.hpp>
#include <asio/read.hpp>
#include <future>
#include <algorithm>

void Tcp_server::start() noexcept {
         if(m_server_running){
                  return;
         }

         m_server_running = true;

         auto worker_thread = [&m_io_context = m_io_context,&m_server_running = m_server_running]{

                  while(m_server_running){
                           m_io_context.run();
                  }
         };

         for(std::uint8_t i = 0;i < m_thread_count;i++){
                  asio::post(m_thread_pool,worker_thread);
         }

         m_logger.server_log("started with",static_cast<std::uint16_t>(m_thread_count),"threads");

         configure_ssl_context();
         configure_acceptor();
         asio::post(m_io_context,[this]{ listen(); });
}

void Tcp_server::shutdown() noexcept {
         if(!m_server_running){
                  return;
         }
         
         m_logger.server_log("shutting down");
         m_server_running = false;
         m_executor_guard.reset();
         m_acceptor.cancel();
         m_acceptor.close(); 
         m_io_context.stop();
         m_thread_pool.join();
         m_logger.server_log("shutdown");
}

void Tcp_server::connection_timeout() noexcept {
         m_acceptor.cancel();
         m_logger.server_log("deaf state");

         auto timeout_timer = std::make_shared<asio::steady_timer>(m_io_context);

         timeout_timer->expires_from_now(std::chrono::seconds(timeout_seconds));

         timeout_timer->async_wait([this,timeout_timer](const auto & error_code){

                  if(!error_code){
                           m_logger.server_log("connection timeout over. shifting to listening state");
                           asio::post(m_io_context,[this]{ listen(); });
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                  }
         });
}

void Tcp_server::shutdown_socket(std::shared_ptr<ssl_tcp_socket> ssl_socket,const std::uint64_t client_id) noexcept {
         {        
                  std::lock_guard client_id_guard(m_client_id_mutex);
                  assert(m_active_client_ids.count(client_id));
                  m_active_client_ids.erase(client_id);
         }

         try{
                  ssl_socket->lowest_layer().shutdown(tcp_socket::shutdown_both);
                  ssl_socket->lowest_layer().close();
         	m_logger.server_log("connection closed with client [",client_id,']');
         }catch(const std::system_error & error){
                  m_logger.error_log(error.what());
         }

         --m_active_connections;
}

void Tcp_server::listen() noexcept {
         m_acceptor.listen();
         m_logger.server_log("listening state");

         if(m_active_connections > max_connections){
                  m_logger.error_log("max connections reached. taking a connection timeout for",timeout_seconds,"seconds");
                  asio::post(m_io_context,[this]{ connection_timeout(); });
                  return;
         }

         using id_task_type = std::packaged_task<std::uint64_t()>;

         auto client_id_task = std::make_shared<id_task_type>([this]{ return get_random_spare_id(); });
         asio::post(m_io_context,[client_id_task]{ return (*client_id_task)(); });

         auto ssl_socket = std::make_shared<ssl_tcp_socket>(m_io_context,m_ssl_context);

         auto on_connection_attempt = [this,ssl_socket,client_id_task](const auto & error_code){

                  if(!error_code){
                           auto new_client_id = client_id_task->get_future().get();

                           {
                                    std::lock_guard client_id_guard(m_client_id_mutex);
                                    m_active_client_ids.insert(new_client_id);
                           }

                           m_logger.server_log("new client [",new_client_id,"] attempting to connect. handshake pending");
                           asio::post(m_io_context,[this,ssl_socket,new_client_id]{ attempt_handshake(ssl_socket,new_client_id); });
                           asio::post(m_io_context,[this]{ listen(); });
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                           // socket could not connect - no shutdown required
                  }
         };

         m_acceptor.async_accept(ssl_socket->lowest_layer(),on_connection_attempt);
}

void Tcp_server::process_message(const Network_message & message,const asio::error_code & connection_code) noexcept {

         auto on_write = [this,ssl_socket = message.ssl_socket,client_id = message.client_id](const auto & error_code,const auto bytes_sent){

                  if(!error_code){
                           m_logger.server_log(bytes_sent,"bytes sent to client [",client_id,']');
                           
                           {
                                    std::shared_lock received_messages_guard(m_received_messages_mutex);

                                    const auto & all_client_messages = m_received_messages[client_id];
                                    assert(m_received_messages.count(client_id));

                                    received_messages_guard.unlock();

                                    m_logger.send_log(client_id,all_client_messages);
                           }
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                  }
                  
                  asio::post(m_io_context,[this,ssl_socket,client_id]{ shutdown_socket(ssl_socket,client_id); });
         };

         m_logger.server_log("processing message from client [",message.client_id,']');
         m_logger.receive_log(message.client_id,*message.content);
         
         {
                  std::lock_guard received_messages_Guard(m_received_messages_mutex);

                  if(m_received_messages.count(message.client_id)){
                           m_received_messages[message.client_id] += *message.content;
                  }else{
                           m_received_messages[message.client_id] = std::move(*message.content);
                  }
         }

         if(connection_code){
                  std::shared_lock messages_guard(m_received_messages_mutex);
                  asio::async_write(*message.ssl_socket,asio::buffer(m_received_messages[message.client_id]),on_write);
         }else{
                  asio::post(m_io_context,[this,ssl_socket = message.ssl_socket,client_id = message.client_id]{ 
                           read_message(ssl_socket,client_id); 
                  });
         }
}

void Tcp_server::read_message(std::shared_ptr<ssl_tcp_socket> ssl_socket,const std::uint64_t client_id) noexcept {

         auto on_read = [this,ssl_socket,client_id](auto read_buffer,const auto & error_code,const auto bytes_read){

                  const auto received_valid_message = [&error_code,bytes_read]{
                           return bytes_read && (!error_code || error_code == asio::error::eof || error_code == asio::error::no_permission);
                  };
                  
                  if(received_valid_message()){
                           asio::post(m_io_context,[this,ssl_socket,read_buffer,client_id,error_code]{
                                    process_message({ssl_socket,read_buffer,client_id},error_code);
                           });
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                           asio::post(m_io_context,[this,ssl_socket,client_id]{ shutdown_socket(ssl_socket,client_id); });
                  }
         };

         auto on_read_wait_over = [this,ssl_socket,client_id,on_read](const auto & error_code){

                  if(!error_code){
                           m_logger.server_log("message received from client [",client_id,']');

                           auto read_buffer = std::make_shared<std::string>(ssl_socket->lowest_layer().available(),'\0');

                           asio::async_read(*ssl_socket,asio::buffer(*read_buffer),[on_read,read_buffer](auto && error_code,auto bytes_read){
                                    on_read(read_buffer,std::forward<decltype(error_code)>(error_code),bytes_read);
                           });
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                           asio::post(m_io_context,[this,ssl_socket,client_id] { shutdown_socket(ssl_socket,client_id); });
                  }
         };

         ssl_socket->lowest_layer().async_wait(tcp_socket::wait_read,on_read_wait_over);
}

void Tcp_server::attempt_handshake(std::shared_ptr<ssl_tcp_socket> ssl_socket,const std::uint64_t client_id) noexcept {
         
         auto on_handshake = [this,ssl_socket,client_id](const auto & error_code){

                  if(!error_code){
                           m_logger.server_log("handshake successful with client [",client_id,']');
                           ++m_active_connections;
                           asio::post(m_io_context,[this,ssl_socket,client_id]{ read_message(ssl_socket,client_id); });
                  }else{
                           m_logger.error_log(error_code,error_code.message());
                           asio::post(m_io_context,[this,ssl_socket,client_id]{ shutdown_socket(ssl_socket,client_id); });
                  }
         };

         m_logger.server_log("handshake attempt with client [",client_id,']');
         ssl_socket->async_handshake(asio::ssl::stream_base::handshake_type::server,on_handshake);
}

