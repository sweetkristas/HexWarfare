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

#include <SDL.h>

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
						std::cerr << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << "\n";
						event.peer->data = "Client information";
						break;
					case ENET_EVENT_TYPE_RECEIVE:
						std::cerr 
							<< "A packet of length " 
							<< event.packet->dataLength 
							<< " containing " 
							<< std::string(reinterpret_cast<char*>(event.packet->data), event.packet->dataLength)
							<< " was received from " 
							<< reinterpret_cast<char*>(event.peer->data) 
							<< " on channel " 
							<< event.channelID
							<< "\n";
						enet_packet_destroy(event.packet);
						break;
					case ENET_EVENT_TYPE_DISCONNECT:
						std::cerr << reinterpret_cast<char*>(event.peer->data) << " disconnected.\n";
						event.peer->data = nullptr;
						break;
				}
			}
		}
	}

	client::client(const std::string& address, int port, int down_bw, int up_bw)
		: address_(address),
		  port_(port),
		  channels_(2),
		  downstream_bandwidth_(down_bw),
		  upstream_bandwidth_(up_bw),
		  connect_timeout_(10),
		  running_(true),
		  thread_(nullptr),
		  lock_(nullptr)
	{
		lock_ = SDL_CreateMutex();
		ASSERT_LOG(lock_ != nullptr, "Unable to create locking mutex");

		std::cerr << "Creating client.\n";
		client_ = enet_host_create(nullptr, 1, channels_, downstream_bandwidth_, upstream_bandwidth_);
		ASSERT_LOG(client_ != nullptr, "An error occurred while trying to create an ENet client host.");

		ENetAddress addr;
		enet_address_set_host(&addr, address_.c_str());
		addr.port = port_;

		std::cerr << "Connecting to peer.\n";
		peer_ = enet_host_connect(client_, &addr, channels_, 0);
		ASSERT_LOG(peer_ != nullptr, "No available peers for initiating an ENet connection.");

		std::cerr << "Creating client communications thread.\n";
		thread_ = SDL_CreateThread(&client::run, "enet_client", this);
		ASSERT_LOG(thread_ != nullptr, "Unable to create enet_client thread.");
	}

	client::~client()
	{
		stop();
		SDL_DestroyMutex(lock_);		
		enet_host_destroy(client_);
	}

	void client::process()
	{
	}

	bool client::is_running() 
	{
		bool running = false;
		if(SDL_LockMutex(lock_) == 0) {
			running = running_;
			SDL_UnlockMutex(lock_);
		} else {
			ASSERT_LOG(false, "Unable to lock mutex");
		}
		return running;
	}

	void client::stop()
	{
		if(SDL_LockMutex(lock_) == 0) {
			running_ = false;
			SDL_UnlockMutex(lock_);
		} else {
			ASSERT_LOG(false, "Unable to lock mutex");
		}
		int status;
		SDL_WaitThread(thread_, &status);
	}

	int client::run(void* ptr)
	{
		client* that = reinterpret_cast<client*>(ptr);
		ENetEvent ev;
		while(that->is_running()) {
			if(enet_host_service(that->client_, &ev, that->connect_timeout_) > 0) {
				switch(ev.type) {
				case ENET_EVENT_TYPE_CONNECT:
					std::cerr << "Connected to " << ev.peer->address.host << "\n";
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					std::cerr << "Got message " << ev.packet->dataLength << " bytes long\n";
					// XXX buffer message here
					enet_packet_destroy(ev.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					std::cerr << "Disconnected " << reinterpret_cast<char*>(ev.peer->data) << "\n";
					break;
				}
			}

			// XXX check send queue and send messages here
			std::string message("Hello, world!");
			ENetPacket *packet = enet_packet_create(message.c_str(), message.size(), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(that->peer_, 0, packet);
		}
		return 0;
	}
}
