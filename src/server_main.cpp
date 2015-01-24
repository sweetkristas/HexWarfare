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

#ifdef SERVER_BUILD

#include <vector>
#include <cstdarg>

#include "lua.hpp"
#include <LuaBridge.h>

#include "bot.hpp"
#include "creature.hpp"
#include "enet_server.hpp"
#include "game_state.hpp"
#include "internal_server.hpp"
#include "internal_client.hpp"
#include "network_server.hpp"

//	enet::server enet_server(9000);
//	enet_server.run();

int main(int argc, char* argv[])
{
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.push_back(argv[i]);
	}
}

#endif
