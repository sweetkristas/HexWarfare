/*
   Copyright 2014 Kristina Simpson <sweet.kristas@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "asserts.hpp"
#include "internal_client.hpp"

namespace network
{
	namespace internal
	{
		client::client(game::state& gs) 
			: base(gs)
		{
		}

		void client::add_peer(std::weak_ptr<base> server) 
		{
			server_ = server;
		}

		void client::handle_process() 
		{
			// Basically we transfer packets to the server, by directly writing them
			// into it's receive queue from our send queue.
			auto peer = server_.lock();
			ASSERT_LOG(peer != nullptr, "No server peer set in network::internal::client");
			game::Update* up;
			while((up = read_send_queue()) != nullptr) {
				peer->write_recv_queue(up);
			}
		}
	}
}