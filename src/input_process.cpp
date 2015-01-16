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

#include "SDL.h"

#include "component.hpp"
#include "easing_between_points.hpp"
#include "input_process.hpp"

namespace process
{
	input::input()
		: process(ProcessPriority::input),
		  state_(State::IDLE),
		  max_opponent_count_(0)
	{
	}

	input::~input()
	{
	}

	bool input::handle_event(const SDL_Event& evt)
	{
		switch(evt.type) {
			case SDL_KEYDOWN:
				keys_pressed_.push(evt.key.keysym.scancode);
				return true;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				mouse_button_events_.push(evt.button);
				break;
			case SDL_MOUSEMOTION:
				mouse_motion_events_.push(evt.motion);
				return true;
		}
		return false;
	}

	void input::update(engine& eng, double t, const entity_list& elist)
	{
		static component_id input_mask = genmask(Component::INPUT);
		static component_id pos_mask = genmask(Component::POSITION) | genmask(Component::STATS);

		for(auto& e : elist) {
			if((e->mask & pos_mask) == pos_mask && (e->mask & input_mask) == input_mask) {
				auto& pos = e->pos.gs_pos;
				auto& inp = e->inp;

				if(inp->clear_selection) {
					inp->selected = false;
					inp->possible_moves.clear();
					inp->graph.reset();
					inp->arrow_path.clear();
					inp->tile_path.clear();
					inp->clear_selection = false;
				}
				if(inp->gen_moves) {
					inp->gen_moves = false;	
					inp->graph = hex::create_cost_graph(eng.get_game_state(), pos.x, pos.y, e->stat->move);
					inp->possible_moves = hex::find_available_moves(inp->graph, pos, e->stat->move);
					// remove tiles that have friendly entities on them, from the results.
					// XXX this needs to be incorporated into hex::find_available_moves somehow.
					inp->possible_moves.erase(std::remove_if(inp->possible_moves.begin(), inp->possible_moves.end(), [&elist](const hex::move_cost& mc) {
						for(auto& e : elist) {
							if(e->pos.gs_pos == mc.loc) {
								return true;
							}
						}
						return false;
					}), inp->possible_moves.end());

					inp->arrow_path.clear();
				}
			}
		}

		// Process keystrokes
		while(!keys_pressed_.empty()) {
			auto key = keys_pressed_.front();
			keys_pressed_.pop();
			if(key == SDL_SCANCODE_E) {
				eng.end_turn();
			} else if(key == SDL_SCANCODE_1) {
				aggressor_ = eng.get_game_state().get_entities().front();
				// XXX This assert may need to be a user error.
				ASSERT_LOG(aggressor_ != nullptr, "No unit on list with which to attack with.");
				if(aggressor_->stat->attacks_this_turn > 0) {
					// Scan through list of enemy entities and select ones which are in 
					// range for being attacked.
					bool opponent_in_range = false;
					for(auto& e2 : elist) {
						if((e2->mask & pos_mask) == pos_mask && (e2->mask & input_mask) == input_mask) {
							if(eng.get_game_state().is_attackable(aggressor_, e2)) {
								e2->inp->is_attack_target = true;
								opponent_in_range = true;
							}
						}
					}
					if(opponent_in_range) {
						state_ = State::SELECT_OPPONENTS;
						max_opponent_count_ = 1; // XXX attacking_unit->stat->max_attack_opponents
					}
				}
			}
		}
		if(!mouse_button_events_.empty()) {
			auto button = mouse_button_events_.front(); mouse_button_events_.pop();

			bool mouse_in_entity = false;
			bool mouse_up_event = false;

			for(auto& e : elist) {
				if((e->mask & pos_mask) == pos_mask && (e->mask & input_mask) == input_mask) {
					auto& pos = e->pos.gs_pos;
					auto& inp = e->inp;
					auto& stats = e->stat;

					auto pp = hex::hex_map::get_pixel_pos_from_tile_pos(pos.x, pos.y);
					if(button.button == SDL_BUTTON_LEFT 
						&& button.type == SDL_MOUSEBUTTONUP) {
						mouse_up_event = true;
						bool mouse_in_area = geometry::pointInRect(point(button.x, button.y), inp->mouse_area + pp);
						mouse_in_entity = mouse_in_area || mouse_in_entity;
						if(state_ == State::SELECT_OPPONENTS) {
							if(max_opponent_count_ != 0 
								&& mouse_in_area 
								&& inp->is_attack_target) {
								targets_.emplace_back(e);
								if(--max_opponent_count_ == 0) {
									do_attack_message(eng);
									// XXX Start playing attack animation.
									state_ = State::IDLE;
									aggressor_ = nullptr;
									targets_.clear();
									for(auto& en : elist) {
										if((en->mask & genmask(Component::INPUT)) == genmask(Component::INPUT)) {
											en->inp->is_attack_target = false;
										}
									}
								}
							}
							continue;
						}

						if(mouse_in_area) {
							// Clear old selections.
							for(auto& e : eng.get_entities()) {
								if((e->mask & genmask(Component::INPUT)) == genmask(Component::INPUT)) {
									e->inp->selected = false;
								}
							}
							inp->selected = true;
						} else {
							// Test whether point is in inp->possible_moves(...) and is players turn, then we animate moving the entity to
							// that position, clear the moves and decrement the units movement allowance.
							auto owner = e->owner.lock();
							auto tp = hex::hex_map::get_tile_pos_from_pixel_pos(button.x, button.y);
							auto it = std::find_if(inp->possible_moves.begin(), inp->possible_moves.end(), [&tp](hex::move_cost const& mc){
								return tp == mc.loc;
							});
							if(eng.get_active_player() == owner && eng.get_game_state().get_entities().front() == e && stats->move > FLT_EPSILON && it != inp->possible_moves.end()) {
								ASSERT_LOG(!inp->tile_path.empty(), "tile path was empty.");
								for(auto& t : inp->tile_path) {
									auto tile = eng.get_map()->get_tile_at(t.x, t.y);
									ASSERT_LOG(tile != nullptr, "No tile exists at point: " << pp);
									LOG_DEBUG("tile" << t << ": " << tile->tile()->id() << " : " << tile->tile()->get_cost());
								}
								// Generate an update move message.
								auto up = eng.get_game_state().create_update();
								eng.get_game_state().unit_move(up, e, inp->tile_path);
								// send message to server.
								auto netclient = eng.get_netclient().lock();
								ASSERT_LOG(netclient != nullptr, "Network client has gone away.");
								netclient->write_send_queue(up);

								auto old_pos = e->pos.pos;
								eng.add_animated_property("unit", std::make_shared<property::animate<double, point>>(
									[old_pos, tp](double t, double d){ return easing::between::linear_tween(t, old_pos, tp, d); }, 
									[e](const point& v){ e->pos.pos = v; }, 2.5));
								//pos.x = tp.x;
								//pos.y = tp.y;

								// decrement movement.
								stats->move -= it->path_cost;
								if(stats->move < FLT_EPSILON) {
									stats->move = 0.0f;
								} else {
									inp->gen_moves = true;
								}
							}
						}
					}
				}
			}

			// This handles clearing the current target(s) if we click on somewhere else on-screen
			if(!mouse_in_entity && state_ == State::SELECT_OPPONENTS && mouse_up_event) {
				state_ = State::IDLE;
				aggressor_ = nullptr;
				targets_.clear();
				for(auto& en : elist) {
					if((en->mask & genmask(Component::INPUT)) == genmask(Component::INPUT)) {
						en->inp->is_attack_target = false;
					}
				}
			}
		}

		if(!mouse_motion_events_.empty()) {
			auto motion = mouse_motion_events_.front(); mouse_motion_events_.pop();
			for(auto& e : elist) {
				if((e->mask & pos_mask) == pos_mask && (e->mask & input_mask) == input_mask) {
					auto& pos = e->pos.gs_pos;
					auto& inp = e->inp;
					auto& stats = e->stat;
					if(!inp->possible_moves.empty() && inp->graph != nullptr) {
						int x = motion.x;
						int y = motion.y;
						auto destination_pt = eng.get_map()->get_tile_pos_from_pixel_pos(x, y);
						if(eng.get_map()->get_tile_at(destination_pt.x, destination_pt.y)) {
							auto it = std::find_if(inp->possible_moves.begin(), inp->possible_moves.end(), [&destination_pt](hex::move_cost const& mc){
								return destination_pt == mc.loc;
							});

							if(it != inp->possible_moves.end()) {
								inp->tile_path = std::move(hex::find_path(inp->graph, pos, destination_pt));
								inp->arrow_path.clear();
								for(auto& t : inp->tile_path) {
									auto p = hex::hex_map::get_pixel_pos_from_tile_pos(t.x, t.y) + point(eng.get_tile_size().x/2, eng.get_tile_size().y/2);
									inp->arrow_path.emplace_back(p);
								}
							}
						}
					}
				}
			}
		}
	}

	void input::do_attack_message(engine& eng)
	{
		if(targets_.empty()) {
			LOG_ERROR("No targets selected for attack!");
			return;
		}
		std::stringstream ss;
		bool first = true;
		for(auto& t : targets_) {
			ss << (first ? " " : "," ) << t->stat->name << "(" << t->entity_id << ")";
			if(first) {
				first = false;
			}
		}
		LOG_INFO("Unit " << aggressor_->stat->name << "(" << aggressor_->entity_id << ") attacks units:" << ss.str());
		// Generate an update move message.
		auto up = eng.get_game_state().create_update();
		eng.get_game_state().unit_attack(up, aggressor_, targets_);
		// send message to server.
		auto netclient = eng.get_netclient().lock();
		ASSERT_LOG(netclient != nullptr, "Network client has gone away.");
		netclient->write_send_queue(up);
	}
}
