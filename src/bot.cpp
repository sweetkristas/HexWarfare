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
#include "component.hpp"
#include "game_state.hpp"
#include "hex_logical_tiles.hpp"
#include "hex_pathfinding.hpp"
#include "message_format.pb.h"
#include "profile_timer.hpp"
#include "random.hpp"

namespace ai
{
	void local_bot_code(player_ptr bot, game::state gs, network::client_ptr client)
	{
		game::Update* up;
		bool running = true;
		profile::timer time;
		while(running) {
			if((up = client->read_recv_queue()) != nullptr) {
				bool end_of_turn_message = false;
				std::cerr << "local_bot_code: Got message: " << up->id() << "\n";
				gs.apply(up);
				if(up->has_quit() && up->quit() == true && up->id() == -1) {
					running = false;
				}
				if(up->has_end_turn()) {
					end_of_turn_message = up->end_turn();
				}
				delete up;

				if(end_of_turn_message) {
					up = bot->process(gs, time.get_time());
					if(up) {
						client->write_send_queue(up);
					}
				}
			}

			client->process();

			profile::sleep(0.01);
		}
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
		auto& e = gs.get_entities().front();

		// First test is whether the currently active entity is ours.
		if(e->owner.lock()->get_uuid() != get_uuid()) {
			return nullptr;
		}

		LOG_DEBUG("Running bot for " << e);

		// Find available moves for current unit.
		auto g = hex::create_cost_graph(gs, e->pos->gs_pos.x, e->pos->gs_pos.y, e->stat->move);
		auto possible_moves = hex::find_available_moves(g, e->pos->gs_pos, e->stat->move);

		// Calculate the closest enemy and move towards them
		component_set_ptr closest_enemy = nullptr;
		int closest_distance = std::numeric_limits<int>::max();
		for(auto& enemy : gs.get_entities()) {
			// XXX we should come up with a faster access for the team id, maybe add it directly as a member of component_set_ptr
			if(enemy->owner.lock()->team() != e->owner.lock()->team()) {
				if(closest_enemy == nullptr) {
					closest_enemy = enemy;
					closest_distance = hex::logical::distance(e->pos->gs_pos, enemy->pos->gs_pos);
				} else {
					int d = hex::logical::distance(e->pos->gs_pos, enemy->pos->gs_pos);
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
		if(closest_enemy) {
			auto surrounds = gs.get_map()->get_surrounding_positions(closest_enemy->pos->gs_pos);
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
				rp = hex::find_path(g, e->pos->gs_pos, dest);
			} else {
				// Nope. Then we need to find the tile that is closest and move there.
				int closest_d = std::numeric_limits<int>::max();
				point closest_pos;
				for(auto& p : possible_moves) {
					int d = hex::logical::distance(p.loc, closest_enemy->pos->gs_pos);
					if(d < closest_d) {
						closest_d = d;
						closest_pos = p.loc;
					}
				}
				rp = hex::find_path(g, e->pos->gs_pos, closest_pos);
			}
		}

		ASSERT_LOG(got_location, "Programmer error: No location for destination.");

		// Choose random destination
		//int x = generator::get_uniform_int<int>(0, static_cast<int>(possible_moves.size()));
		//auto rp = hex::find_path(g, e->pos->gs_pos, possible_moves[x].loc);

		// Random move unit.
		game::Update* up = gs.create_update();
		gs.unit_move(up, e, rp);
		
		// See if there is anything in attack range.
		std::vector<component_set_ptr> attackable;
		int max_attacks = e->stat->unit->get_max_units_attackable();
		for(auto& ae : gs.get_entities()) {
			if(gs.is_attackable(e, ae)) {
				attackable.emplace_back(ae);
				if(--max_attacks == 0) {
					break;
				}
			}
		}
		if(!attackable.empty()) {
			gs.unit_attack(up, e, attackable);
		}
		gs.end_turn(up);
		return up;
	}

	player_ptr bot::clone()
	{
		return std::shared_ptr<bot>(new bot(*this));
	}

}
