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


#include "network_server.hpp"

namespace network
{
	server_base::server_base()
	{
	}

	server_base::~server_base()
	{
	}

	void server_base::process()
	{
		handle_process();
	}

	void server_base::send_packet(Update* up)
	{
		snd_q_.push(up);
	}

	Update* server_base::receive_packet()
	{
		Update* up;
		if(rcv_q_.try_pop(up)) {
			return up;
		}
		return nullptr;
	}
}
