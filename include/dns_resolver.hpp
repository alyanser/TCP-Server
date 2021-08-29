#ifndef DNS_RESOLVER_HPP
#define DNS_RESOLVER_HPP
#pragma once

#include <asio/ip/tcp.hpp>
#include <string>

namespace asio {
         class io_context;
}

class dns_resolver {
         using result_type = asio::ip::tcp::resolver::iterator;
public:
         dns_resolver(asio::io_context & io_context);

         result_type resolve(const std::string & hostname,const std::string & port);
private:
         asio::ip::tcp::resolver m_resolver;
};

#endif