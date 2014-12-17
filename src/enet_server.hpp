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

#pragma once

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <enet/enet.h>

#include "message_format.pb.h"
#include "mutex.hpp"
#include "queue.hpp"
#include "threads.hpp"

namespace enet
{
	class server
	{
	public:
		explicit server(int port);
		~server();
		void run();
	private:
		int port_;
		bool running_;

		std::deque<game::Update*> send_q_;
		std::deque<game::Update*> recv_q_;

		static void signal_handler(int signal_number);

		std::map<int, ENetPeer*> peers_;

		server() = delete;
		server(const server&) = delete;
		void operator=(const server&) = delete;
	};

	class client
	{
	public:
		explicit client(const std::string& address, int port, int down_bw=0, int up_bw=0);
		~client();
		void process();
		void send_data(game::Update*);
		game::Update* get_pending_packet();
	private:
		std::string address_;
		int port_;
		int channels_;
		int downstream_bandwidth_;
		int upstream_bandwidth_;
		int connect_timeout_;
		bool running_;

		enum class state
		{
			CONNECTING,
			CONNECTED,
		};

		ENetHost* client_;
		ENetPeer* peer_;

		int run();
		void stop();
		bool is_running();

		threading::Mutex mutex_;
		std::unique_ptr<threading::Thread> thread_;

		queue::queue<game::Update*> send_q_;
		queue::queue<game::Update*> rcv_q_;

		client() = delete;
		client(const client&) = delete;
		void operator=(const client&) = delete;
	};
}
