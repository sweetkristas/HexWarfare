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

		server();
		server(const server&);
		void operator=(const server&);
	};
}
