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

#include <csignal>
#include <memory>

#include "asserts.hpp"
#include "enet_server.hpp"

namespace enet
{
	bool server_running = true;
	threading::Mutex& get_mutex()
	{
		static threading::Mutex res;
		return res;
	}

	bool is_server_running()
	{
		threading::Mutex::Lock lock(get_mutex());
		return server_running;
	}

	void stop_server()
	{
		threading::Mutex::Lock lock(get_mutex());
		server_running = false;
	}

	server::server(int port)
		: port_(port),
		  running_(false)
	{
		ASSERT_LOG(enet_initialize() == 0, "An error occurred while initializing ENet.");
	}

	server::~server()
	{
		enet_deinitialize();
	}

	void server::signal_handler(int signal_number)
	{
		std::cerr << "Got signal " << signal_number << "\n";
		//running_ = false;
		stop_server();
	}

	void server::run()
	{
		ENetAddress address = { ENET_HOST_ANY, static_cast<unsigned short>(port_) };
		// up to 32 clients, 2 channels, any amount incoming bandwidth, any amount of outgoing bandwidth.
		std::shared_ptr<ENetHost> e_server(enet_host_create (&address, 32, 2, 0, 0), enet_host_destroy);
		ASSERT_LOG(e_server != nullptr, "An error occurred while trying to create an ENet server host.");

		signal(SIGTERM, signal_handler);
		signal(SIGINT, signal_handler);

		ENetEvent ev;
		game::Update up;
		static int peer_cnt = 0;
		running_ = true;
		while(is_server_running()) {
			if(enet_host_service (e_server.get(), &ev, 0) > 0) {
				switch(ev.type) {
					case ENET_EVENT_TYPE_CONNECT: {
						std::cerr << "A new client connected from " << ev.peer->address.host << ":" << ev.peer->address.port << "\n";
						peers_[peer_cnt] = ev.peer;
						ev.peer->data = reinterpret_cast<void*>(peer_cnt);
						peer_cnt++;
						break;
					}
					case ENET_EVENT_TYPE_RECEIVE: {
						std::string pkt(reinterpret_cast<char*>(ev.packet->data), ev.packet->dataLength);
						up.Clear();
						up.ParseFromString(pkt);
						std::cerr 
							<< "A packet of length " 
							<< ev.packet->dataLength 
							<< " containing " 
							<< up.id()
							<< " was received from " 
							<< reinterpret_cast<intptr_t>(ev.peer->data) 
							<< " on channel " 
							<< ev.channelID
							<< "\n";
						enet_packet_destroy(ev.packet);
						break;
					}
					case ENET_EVENT_TYPE_DISCONNECT: {
						int peer_value = reinterpret_cast<intptr_t>(ev.peer->data);
						auto it = peers_.find(peer_value);
						if(it != peers_.end()) {
							std::cerr << peer_value << " disconnected.\n";
							peers_.erase(it);
							ev.peer->data = nullptr;
						}
						break;
					}
					default: break;
				}
			}
		}

		for(auto p : peers_) {
			enet_peer_disconnect(p.second, 0);
			ENetEvent ev;
			bool disconnect_ok = false;
			while(enet_host_service(e_server.get(), &ev, 0) > 0) {
				switch(ev.type) {
					case ENET_EVENT_TYPE_RECEIVE: 
						enet_packet_destroy(ev.packet);
						break;
					case ENET_EVENT_TYPE_DISCONNECT: 
						disconnect_ok = true;
						break;
					default: break;
				}
			}
			if(!disconnect_ok) {
				enet_peer_reset(p.second);
			}
		}
		peers_.clear();
	}

	client::client(const std::string& address, int port, int down_bw, int up_bw)
		: address_(address),
		  port_(port),
		  channels_(2),
		  downstream_bandwidth_(down_bw),
		  upstream_bandwidth_(up_bw),
		  connect_timeout_(20),
		  running_(true),
		  mutex_()
	{
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
#ifdef _MSC_VER
		thread_ = std::make_unique<threading::Thread>("enet_client", std::bind(&client::run, this));
#else
		thread_ = std::unique_ptr<threading::Thread>(new threading::Thread("enet_clent", std::bind(&client::run, this)));
#endif
		ASSERT_LOG(thread_ != nullptr, "Unable to create enet_client thread.");
	}

	client::~client()
	{
		stop();
		enet_host_destroy(client_);
	}

	void client::process()
	{
	}

	bool client::is_running() 
	{
		threading::Mutex::Lock lock(mutex_);
		return running_;
	}

	void client::stop()
	{
		{
			threading::Mutex::Lock lock(mutex_);
			running_ = false;
		}
		thread_->join();
	}

	void client::send_data(game::Update* snd)
	{
		send_q_.push(snd);
	}

	game::Update* client::get_pending_packet()
	{
		game::Update* up = nullptr;
		if(rcv_q_.try_pop(up)) {
			return up;
		}
		return nullptr;
	}

	int client::run()
	{
		ENetEvent ev;
		bool connected = false;
		while(is_running()) {
			if(enet_host_service(client_, &ev, connect_timeout_) > 0) {
				switch(ev.type) {
				case ENET_EVENT_TYPE_CONNECT:
					std::cerr << "Connected to " << ev.peer->address.host << "\n";
					connected = true;
					break;
				case ENET_EVENT_TYPE_RECEIVE: {
					std::cerr << "Got message " << ev.packet->dataLength << " bytes long\n";
					std::string pkt(reinterpret_cast<char*>(ev.packet->data), ev.packet->dataLength);
					game::Update* up = new game::Update();
					up->ParseFromString(pkt);
					rcv_q_.push(up);
					enet_packet_destroy(ev.packet);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
					std::cerr << "Disconnected " << (ev.peer->data != nullptr ? reinterpret_cast<char*>(ev.peer->data) : "null") << "\n";
					connected = false;
					break;
				default: break;
				}
			}

			// XXX check send queue and send messages here
			if(!send_q_.empty() && connected) {
				game::Update* msg;
				if(send_q_.wait_and_pop(msg)) {
					static std::string message;
					msg->SerializeToString(&message);
					ENetPacket *packet = enet_packet_create(message.c_str(), message.size(), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer_, 0, packet);
					delete msg;
				}
			}
		}
		return 0;
	}
}
