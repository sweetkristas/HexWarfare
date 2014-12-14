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
#include <memory>
#include <string>
#include <vector>

#include <SDL.h>
#include <enet/enet.h>

#include "node.hpp"

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

		std::deque<node> send_q_;
		std::deque<node> recv_q_;

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

		static int run(void*);
		void stop();
		bool is_running();

		SDL_Thread *thread_;
		SDL_mutex* lock_;

		//std::deque<...> send_q_;
		//std::deque<...> rcv_q_;

		client() = delete;
		client(const client&) = delete;
		void operator=(const client&) = delete;
	};
}
