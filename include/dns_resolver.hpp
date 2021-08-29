#ifndef DNS_RESOLVER_HPP
#define DNS_RESOLVER_HPP
#pragma once

#include <asio/ip/tcp.hpp>
#include <string>

namespace asio {
         class io_context;
}

namespace std {
         class mutex;
}

class dns_resolver {
         using result_type = asio::ip::tcp::resolver::iterator;
public:
         dns_resolver(asio::io_context & io_context);
         dns_resolver(const dns_resolver & rhs) = delete;
         dns_resolver(dns_resolver & rhs) = delete;

         result_type resolve(const std::string & hostname,const std::string & port);
private:
         asio::ip::tcp::resolver m_resolver;
};

#endif