#include <asio/io_context.hpp>
#include <asio/ip/basic_resolver.hpp>
#include <dns_resolver.hpp>

dns_resolver::dns_resolver(asio::io_context & io_context) : m_resolver(io_context){}

dns_resolver::result_type dns_resolver::resolve(const std::string & hostname,const std::string & port){
         asio::ip::tcp::resolver::query query(hostname,port);
         return m_resolver.resolve(query);
}