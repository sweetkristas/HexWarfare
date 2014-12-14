//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once 

#include <array>
#include <memory>
#include <boost/asio.hpp>
#include "reply.hpp"
#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"

namespace http 
{
    namespace server 
    {
        class connection_manager;

        /// Represents a single connection from a client.
        class connection
          : public std::enable_shared_from_this<connection>
        {
        public:
          /// Construct a connection with the given socket.
          explicit connection(boost::asio::ip::tcp::socket socket, connection_manager& manager, request_handler& handler);

          /// Start the first asynchronous operation for the connection.
          void start();

          /// Stop all asynchronous operations associated with the connection.
          void stop();

        private:
          /// Perform an asynchronous read operation.
          void doRead();

          /// Perform an asynchronous write operation.
          void doWrite();

          /// Socket for the connection.
          boost::asio::ip::tcp::socket socket_;

          /// The manager for this connection.
          connection_manager& connection_manager_;

          /// The handler used to process the incoming request.
          request_handler& request_handler_;

          /// Buffer for incoming data.
          std::array<char, 8192> buffer_;

          /// The incoming request.
          request request_;

          /// The parser for the incoming request.
          request_parser request_parser_;

          /// The reply to be sent back to the client.
          reply reply_;

          connection(const connection&) = delete;
          connection& operator=(const connection&) = delete;         
        };

        typedef std::shared_ptr<connection> connection_ptr;
    }
}