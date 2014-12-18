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
#include "internal_server.hpp"

namespace network
{
	namespace internal
	{
		server::server()
		{
		}
		
		void server::add_peer(std::weak_ptr<base> client)
		{
			clients_.emplace_back(client);
		}

		void server::handle_process()
		{
			// remove any clients which have died
			clients_.erase(std::remove_if(clients_.begin(), clients_.end(), [](std::weak_ptr<base> p){ return p.lock() == nullptr; }), clients_.end());

			// Take messages from our send queue and send them to each connected client.
			game::Update* up;
			while((up = read_send_queue()) != nullptr) {
				for(auto& c : clients_) {
					auto peer = c.lock();
					ASSERT_LOG(peer != nullptr, "client has gone away, peer == nullptr");
					peer->write_recv_queue(new game::Update(*up));
				}
				delete up;
			}
		}
	}
}
