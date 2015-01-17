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

#include <limits>

#include "asserts.hpp"
#include "bot.hpp"
#include "creature.hpp"
#include "game_state.hpp"
#include "hex_logical_tiles.hpp"
#include "hex_pathfinding.hpp"
#include "message_format.pb.h"
#include "profile_timer.hpp"
#include "random.hpp"
#include "units.hpp"

namespace ai
{
	void local_bot_code(player_ptr bot, game::state gs, network::client_ptr client)
	{
		game::Update* up;
		bool running = true;
		profile::timer time;
		while(running) {
			if((up = client->read_recv_queue()) != nullptr) {
				bool fire_process = false;
				std::cerr << "local_bot_code: Got message: " << up->id() << "\n";
				gs.apply(up);
				if(up->has_quit() && up->quit() == true && up->id() == -1) {
					running = false;
				}
				if(up->has_end_turn()) {
					fire_process = up->end_turn();
				}
				if(up->has_game_start()) {
					fire_process = up->game_start();
				}
				if(up->has_game_win_state() && up->game_win_state() != game::Update_GameWinState_IN_PROGRESS) {
					// Bot is dispassionate and exits out after the game is over.
					running = false;
					/// XXX should we send a player quits message here?
				}
				delete up;

				if(fire_process && running) {
					up = bot->process(gs, time.get_time());
					if(up) {
						client->write_send_queue(up);
					}
				}
			}

			// Do network message processing.
			client->process();

			profile::sleep(0.01);
		}
		LOG_INFO("bot exits -- player " << bot->name());
	}

	bot::bot(team_ptr team, const std::string& name, uuid::uuid u)
		: player(team, PlayerType::AI, name, u)
	{
	}

	game::Update* bot::process(const game::state& gs, double time)
	{
		profile::manager botman("Bot process time");
		// Look at game state and decide if we need to do stuff.

		// Get current entity
		auto& u = gs.get_entities().front();

		// First test is whether the currently active entity is ours.
		if(u->get_owner()->get_uuid() != get_uuid()) {
			return nullptr;
		}

		LOG_DEBUG("Running bot for " << u);

		// Find available moves for current unit.
		auto g = hex::create_cost_graph(gs, u->get_position(), u->get_move());
		auto possible_moves = hex::find_available_moves(g, u->get_position(), u->get_move());

		// Calculate the closest enemy and move towards them
		game::unit_ptr closest_enemy = nullptr;
		int closest_distance = std::numeric_limits<int>::max();
		for(auto& enemy : gs.get_entities()) {
			// XXX we should come up with a faster access for the team id, maybe add it directly as a member of component_set_ptr
			if(enemy->get_owner()->team() != u->get_owner()->team()) {
				if(closest_enemy == nullptr) {
					closest_enemy = enemy;
					closest_distance = hex::logical::distance(u->get_position(), enemy->get_position());
				} else {
					int d = hex::logical::distance(u->get_position(), enemy->get_position());
					if(d < closest_distance) {
						closest_enemy = enemy;
						closest_distance = d;
					}
				}
			}
		}
		// Found an enemy, try and get as close as possible.
		point dest;
		bool got_location = false;
		hex::result_path rp;
		if(closest_enemy && closest_distance > u->get_range()) {
			auto surrounds = gs.get_map()->get_surrounding_positions(closest_enemy->get_position());
			for(auto& p : possible_moves) {
				for(auto& sp : surrounds) {
					if(p.loc == sp) {
						// We found a match.
						got_location = true;
						dest = sp;
						break;
					}
				}
			}

			// Did we find a location of enemy within our list of possible moves?
			if(got_location) {
				rp = hex::find_path(g, u->get_position(), dest);
			} else {
				// Nope. Then we need to find the tile that is closest and move there.
				int closest_d = std::numeric_limits<int>::max();
				point closest_pos;
				for(auto& p : possible_moves) {
					int d = hex::logical::distance(p.loc, closest_enemy->get_position());
					if(d < closest_d) {
						closest_d = d;
						closest_pos = p.loc;
					}
				}
				rp = hex::find_path(g, u->get_position(), closest_pos);
				got_location = true;
			}
		}

		ASSERT_LOG(got_location || closest_distance <= u->get_range(), "Programmer error: No location for destination.");

		// Choose random destination
		//int x = generator::get_uniform_int<int>(0, static_cast<int>(possible_moves.size()));
		//auto rp = hex::find_path(g, e->pos.gs_pos, possible_moves[x].loc);

		// Random move unit.
		game::Update* up = gs.create_update();
		// No need to move if there is a unit next to us.
		if(closest_distance > u->get_range()) {
			gs.unit_move(up, u, rp);
		}
		
		// See if there is anything in attack range.
		//while(e->stat->attacks_this_turn > 0) {
			std::vector<game::unit_ptr> attackable;
			int max_attacks = u->get_type()->get_max_units_attackable();
			for(auto& ae : gs.get_entities()) {
				if(gs.is_attackable(u, ae)) {
					attackable.emplace_back(ae);
					if(--max_attacks == 0) {
						break;
					}
				}
			}
			if(!attackable.empty()) {
				gs.unit_attack(up, u, attackable);
			//} else {
			//	break;
			}
		//}
		gs.end_turn(up);
		return up;
	}

	player_ptr bot::clone()
	{
		return std::shared_ptr<bot>(new bot(*this));
	}

}
