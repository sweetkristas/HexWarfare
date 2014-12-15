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

#include "message_format.pb.h"
#include "queue.hpp"

namespace network
{
	class server_base
	{
	public:
		server_base();
		virtual ~server_base();

		void process();

		void send_packet(Update*);
		Update* receive_packet();
	private:
		queue::queue<Update*> snd_q_;
		queue::queue<Update*> rcv_q_;

		virtual void handle_process() = 0;

		server_base(const server_base&) = delete;
		void operator=(const server_base&) = delete;
	};
}
