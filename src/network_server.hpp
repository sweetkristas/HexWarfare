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

#include <memory>

#include "message_format.pb.h"
#include "queue.hpp"

namespace network
{
	class base
	{
	public:
		base();
		virtual ~base();

		void process();

		void write_send_queue(game::Update*);
		game::Update* read_recv_queue();

		void write_recv_queue(game::Update* up);
		game::Update* read_send_queue();

		virtual void add_peer(std::weak_ptr<base> peer) = 0;
	private:
		queue::queue<game::Update*> snd_q_;
		queue::queue<game::Update*> rcv_q_;

		virtual void handle_process() = 0;

		base(const base&) = delete;
		void operator=(const base&) = delete;
	};

	typedef std::shared_ptr<base> server_ptr;
	typedef std::weak_ptr<base> server_weak_ptr;
	typedef std::shared_ptr<base> client_ptr;
	typedef std::weak_ptr<base> client_weak_ptr;
}
