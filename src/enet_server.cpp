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

#include <enet/enet.h>

#include "asserts.hpp"
#include "enet_server.hpp"

namespace enet
{
	server::server(int port)
		: port_(port)
	{
		ASSERT_LOG(enet_initialize() == 0, "An error occurred while initializing ENet.");
	}

	server::~server()
	{
		enet_deinitialize();
	}

	void server::run()
	{
		ENetAddress address = { ENET_HOST_ANY, port_ };
		// up to 32 clients, 2 channels, any amount incoming bandwidth, any amount of outgoing bandwidth.
		std::shared_ptr<ENetHost> e_server(enet_host_create (&address, 32, 2, 0, 0), enet_host_destroy);
		ASSERT_LOG(e_server != nullptr, "An error occurred while trying to create an ENet server host.");

		ENetEvent event;
		while(1) {
			if(enet_host_service (e_server.get(), &event, 0) > 0) {
				switch(event.type) {
					case ENET_EVENT_TYPE_CONNECT:
					case ENET_EVENT_TYPE_RECEIVE:
					case ENET_EVENT_TYPE_DISCONNECT:
						break;
				}
			}
		};
	}
}
