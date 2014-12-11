//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio.hpp>
#include <string>

#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http 
{
    namespace server 
    {
        /// The top-level class of the HTTP server.
        class server
        {
        public:
          explicit server(const std::string& address, const std::string& port);
          void run();

        private:
          void doAccept();
          void doAwaitStop();
          
          boost::asio::io_service io_service_;
          boost::asio::signal_set signals_;
          boost::asio::ip::tcp::acceptor acceptor_;
          connection_manager connection_manager_;
          boost::asio::ip::tcp::socket socket_;
          request_handler request_handler_;
          
          server(const server&) = delete;
          server& operator=(const server&) = delete;
        };
    }
}
