//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pramga once

#include <set>
#include "connection.hpp"

namespace http 
{
    namespace server 
    {
        /// Manages open connections so that they may be cleanly stopped when the server
        /// needs to shut down.
        class connection_manager
        {
        public:
            /// Construct a connection manager.
            connection_manager();

            /// Add the specified connection to the manager and start it.
            void start(connection_ptr c);

            /// Stop the specified connection.
            void stop(connection_ptr c);

            /// Stop all connections.
            void stopAll();

        private:
            /// The managed connections.
            std::set<connection_ptr> connections_;
            
            connection_manager(const connection_manager&) = delete;
            connection_manager& operator=(const connection_manager&) = delete;
        };
    }
}