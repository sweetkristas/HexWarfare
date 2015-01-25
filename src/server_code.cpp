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

#include "server_code.hpp"

namespace game
{
	void local_server_code(state gs, network::server_ptr server)
	{
		Update* up;
		bool running = true;

		// create and send a start game packet.
		up = gs.create_update();
		up->set_game_start(true);
		// Set starting gold for all players, with player update messages.
		for(auto& p : gs.get_players()) {
			Update_Player* upp = up->add_player();
			upp->set_uuid(uuid::write(p->get_uuid()));
			upp->set_action(Update_Player_Action_UPDATE);
			Update_PlayerInfo* pi = new Update_PlayerInfo();
			// XXX starting gold per player -- should load from scenario.
			pi->set_gold(50);
			upp->set_allocated_player_info(pi);
		}
		server->write_send_queue(up);
		server->process();

		while(running) {
			if((up = server->read_recv_queue()) != nullptr) {
				std::cerr << "local_server_code: Got message: " << up->id() << "\n";
				// XXX do more processing here.
				LOG_DEBUG("SERVER: received packet of " << up->SerializeAsString().size() << " bytes");
				Update* nup = gs.validate_and_apply(up);
				// add some information debugging
				if(nup) {
					LOG_DEBUG("SERVER: Sending packet of " << nup->SerializeAsString().size() << " bytes");
					server->write_send_queue(nup);
				}
				if(up->has_quit() && up->quit() && up->id() == -1) {
					running = false;
				}
				delete up;

				server->process();
			}
		}
	}
}
