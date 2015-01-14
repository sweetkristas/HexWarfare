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

#include <algorithm>

#include "asserts.hpp"
#include "component.hpp"
#include "creature.hpp"
#include "easing.hpp"
#include "engine.hpp"
#include "node_utils.hpp"
#include "profile_timer.hpp"

namespace 
{
	const std::vector<float>& camera_scale_factors()
	{
		static std::vector<float> res{ 4.0f, 2.0f, 1.0f, 0.5f, 0.25f, 0.125f };
		return res;
	}
}


engine::engine(game::state& game_state, graphics::window_manager& wm)
	: game_state_(game_state),
	  state_(EngineState::PLAY),
	  camera_scale_(2),
	  wm_(wm),
	  particles_(wm.get_renderer())
{
}

engine::~engine()
{
}

component_set_ptr engine::add_entity(component_set_ptr e)
{
	static component_id stat_mask 
		= component::genmask(component::Component::STATS) 
		| component::genmask(component::Component::POSITION);
	entity_list_.emplace_back(e);
	std::stable_sort(entity_list_.begin(), entity_list_.end());
	if((e->mask & stat_mask) == stat_mask) {
		game_state_.add_entity(e);
	}
	return e;
}

void engine::remove_entity(component_set_ptr e1)
{
	static component_id stat_mask 
		= component::genmask(component::Component::STATS) 
		| component::genmask(component::Component::POSITION);
	entity_list_.erase(std::remove_if(entity_list_.begin(), entity_list_.end(), [&e1](component_set_ptr e2) {
		return e1 == e2; 
	}), entity_list_.end());
	if((e1->mask & stat_mask) == stat_mask) {
		game_state_.remove_entity(e1);
	}
}

void engine::add_process(process::process_ptr s)
{
	process_list_.emplace_back(s);
	std::stable_sort(process_list_.begin(), process_list_.end(), [](const process::process_ptr& lhs, const process::process_ptr& rhs){
		return lhs->get_priority() < rhs->get_priority();
	});
	s->start();
}

void engine::remove_process(process::process_ptr s)
{
	s->end();
	process_list_.erase(std::remove_if(process_list_.begin(), process_list_.end(), 
		[&s](process::process_ptr sp) { return sp == s; }), process_list_.end());
}

float engine::get_zoom() const
{
	ASSERT_LOG(camera_scale_ >= 0 && camera_scale_ < camera_scale_factors().size(),
		"Camera scale is out of bounds: 0 <= " << camera_scale_ << " < " << camera_scale_factors().size());
	return camera_scale_factors()[camera_scale_];
}

void engine::translate_mouse_coords(SDL_Event* evt)
{
	// transform the absolute mouse co-ordinates to a window-size independent quantity.
	if(evt->type == SDL_MOUSEMOTION) {
		evt->motion.x = static_cast<Sint32>(((evt->motion.x + camera_.x) / get_zoom()));
		evt->motion.y = static_cast<Sint32>(((evt->motion.y + camera_.y) / get_zoom()));
	} else {
		evt->button.x = static_cast<Sint32>(((evt->button.x + camera_.x) / get_zoom()));
		evt->button.y = static_cast<Sint32>(((evt->button.y + camera_.y) / get_zoom()));
	}
}

void engine::process_events()
{
	SDL_Event evt;
	while(SDL_PollEvent(&evt)) {
		bool claimed = false;
		for(auto& w : widgets_) {
			claimed = w->process_events(&evt, claimed);
		}
		switch(evt.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
				translate_mouse_coords(&evt);
				break;
			case SDL_MOUSEWHEEL:
				if(evt.wheel.y > 0) {
					if(--camera_scale_ <= 0) {
						camera_scale_ = 0;
					}
				} else if(evt.wheel.y < 0) {
					if(++camera_scale_ >= camera_scale_factors().size()) {
						camera_scale_ = camera_scale_factors().size()-1;
					}
				}
				break;
			case SDL_QUIT:
				set_state(EngineState::QUIT);
				return;
			case SDL_WINDOWEVENT:
				claimed = true;
				switch(evt.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						wm_.update_window_size();
						break;
					default: break;
				}
				break;
			case SDL_KEYDOWN:
				//std::cerr << "Camera(before): (" << camera_.x << "," << camera_.y << ")\n";
				if(evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					set_state(EngineState::QUIT);
					return;
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_W) {
					camera_.y -= tile_size_.y;
					clip_camera_to_extents();
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_A) {
					camera_.x -= tile_size_.x;
					clip_camera_to_extents();
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_S) {
					camera_.y += tile_size_.y;
					clip_camera_to_extents();
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_D) {
					camera_.x += tile_size_.x;
					clip_camera_to_extents();
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_P) {
					if(SDL_GetModState() & KMOD_CTRL) {
						if(get_state() == EngineState::PLAY) {
							set_state(EngineState::PAUSE);
							LOG_INFO("Set state paused.");
						} else if(get_state() == EngineState::PAUSE) {
							set_state(EngineState::PLAY);
							LOG_INFO("Set state play.");
						}
						claimed = true;
					}
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_T) {
					// test code
					point pos(wm_.width() / 2, wm_.height() / 2);
					node_builder nb;
					nb.add("lifetime", 5.0);
					node_builder em;
					em.add("type", "square");
					///em.add("max_particles", 2000);
					//em.add("circle_radius", 20.0);
					//em.add("emit_random", true);
					em.add("dimensions", 20.0);
					em.add("dimensions", 10.0);
					em.add("particle_lifetime", 2.0);
					em.add("rate", 500.0);
					nb.add("emitter", em.build());
					particles_.add_system(particle::particle_system::create(pos, nb.build()));
				}
				//std::cerr << "Camera(after) : (" << camera_.x << "," << camera_.y << ")\n";
				break;
		}
		if(!claimed) {
			for(auto& e : entity_list_) {
				static component_id gui_mask = component::genmask(component::Component::GUI);
				if((e->mask & gui_mask) == gui_mask) {
					auto& g = e->gui;
					for(auto& w : g->widgets) {
						claimed = w->process_events(&evt, claimed);
					}
				}
			}
			for(auto& s : process_list_) {
				if(s->process_event(evt)) {
					break;
				}
			}
		}
	}
}

void engine::entity_health_check()
{
	static component_id stat_mask 
		= component::genmask(component::Component::STATS) 
		| component::genmask(component::Component::POSITION);
	entity_list_.erase(std::remove_if(entity_list_.begin(), entity_list_.end(), [&](component_set_ptr e) {
		if((e->mask & stat_mask) == stat_mask) {
			if(e->stat->health <= 0) {
				game_state_.remove_entity(e);
				return true;
			}
		}
		return false;
	}), entity_list_.end());
}

bool engine::update(double time)
{
	process_events();
	property_manager_.process(time);
	if(state_ == EngineState::PAUSE || state_ == EngineState::QUIT) {
		return state_ == EngineState::PAUSE ? true : false;
	}

	for(auto& p : process_list_) {
		p->update(*this, time, entity_list_);
	}

	for(auto& w : widgets_) {
		w->update(*this, time);
	}

	particles_.update(static_cast<float>(time));
	particles_.draw();

	// Scan through entity list, remove any with 0 health
	entity_health_check();
	return true;
}

component_set_ptr engine::get_entity_by_uuid(const uuid::uuid& id)
{
	for(auto& e : entity_list_) {
		if(e->entity_id == id) {
			return e;
		}
	}
	ASSERT_LOG(false, "Couldn't find entity with uuid: " << id);
	return nullptr;
}

// Handle the engine side of game::state updates
void engine::process_update(game::Update* up)
{
	using namespace game;

	for(auto& players : up->player()) {
		// XXX deal with stuff
		switch(players.action())
		{
			case Update_Player_Action_CANONICAL_STATE:
			case Update_Player_Action_JOIN:
			case Update_Player_Action_QUIT:
			case Update_Player_Action_CONCEDE:
				break;
			default: 
				ASSERT_LOG(false, "Unrecognised player.action() value: " << players.action());
		}
	}

	for(auto& units : up->units()) {
		switch(units.type()) {
			case Update_Unit_MessageType_CANONICAL_STATE:
				break;
			case Update_Unit_MessageType_SUMMON:
				break;
			case Update_Unit_MessageType_MOVE: {
				auto e = get_entity_by_uuid(uuid::read(units.uuid()));
				if(e->pos->gs_pos != e->pos->pos) {
					// XXX schedule a movement animation, which we fake for now.
					e->pos->pos = e->pos->gs_pos;
				}
				/// XXX clear any pathing related stuff, or at least signal engine to do it in the input process.
				if(e->inp) {
					e->inp->clear_selection = true;
				}
				break;
			}
			case Update_Unit_MessageType_ATTACK: {
				break;
			}
			case Update_Unit_MessageType_SPELL: {
				break;
			}
			case Update_Unit_MessageType_PASS:
				break;
			default: 
				ASSERT_LOG(false, "Unrecognised units.type() value: " << units.type());
		}
	}

	if(up->has_end_turn() && up->end_turn()) {
		auto& ep = game_state_.get_entities().front()->pos->gs_pos;
		auto fp = get_map()->get_pixel_pos_from_tile_pos(ep.x, ep.y);
		fp += point(get_tile_size().x/2 - get_window().width()/2, get_tile_size().y/2 - get_window().height()/2);			
		add_animated_property("camera", 
			std::make_shared<property::animate<double, glm::vec2>>([&, fp](double t, double d){ 
								return easing::ease_out_quad<glm::vec2, float>(t, glm::vec2(static_cast<float>(camera_.x), static_cast<float>(camera_.y)), glm::vec2(static_cast<float>(fp.x-camera_.x), static_cast<float>(fp.y-camera_.y)), d); }, 
								[&](const glm::vec2& v){ camera_.x = static_cast<int>(std::round(v.x)); camera_.y = static_cast<int>(std::round(v.y)); }, 0.4));
	}

	// Check for a game won/drawn/lost
	if(up->has_game_win_state() && up->game_win_state() != Update_GameWinState_IN_PROGRESS) {
		if(up->game_win_state() == Update_GameWinState_DRAW) {
			// XXX Draw some large graphic splash
			LOG_INFO("Game Over -- draw");
		} else {
			ASSERT_LOG(up->has_winning_team_uuid(), "Gane marked as won, but no winning team uuid.");
			// XXX Draw some large graphic splash
			auto winning_uuid = uuid::read(up->winning_team_uuid());
			auto winning_team = game_state_.get_team_from_id(winning_uuid);
			LOG_INFO("Game Over -- Team '" << winning_team->get_team_name() << "' wins");
			if(active_player_->team() == winning_team) {
				LOG_INFO("You win.");
			} else {
				LOG_INFO("You lost.");
			}
		}

		state_ = EngineState::GAME_OVER;
	}
}

void engine::end_turn()
{
	static component_id input_mask = component::genmask(component::Component::INPUT);
	for(auto& e : entity_list_) {
		if((e->mask & input_mask) == input_mask) {
			auto& inp = e->inp;
			// clear out a bunch of stuff from the input component
			// Should probably make this a function on the input component.
			inp->selected = false;
			inp->possible_moves.clear();
			inp->graph.reset();
			inp->arrow_path.clear();
			inp->tile_path.clear();
			inp->clear_selection = false;
			inp->is_attack_target = false;
		}
	}

	auto netclient = get_netclient().lock();
	ASSERT_LOG(netclient != nullptr, "Network client has gone away.");
	game::Update* up = game_state_.create_update();
	game_state_.end_turn(up);
	netclient->write_send_queue(up);
}

void engine::set_extents(const rect& extents) 
{ 
	extents_ = rect(extents.x(), 
		extents.y(), 
		(extents.w() * 3 * tile_size_.x)/4 + tile_size_.x/4,
		extents.h() * tile_size_.y + tile_size_.y/2);
}

const rect& engine::get_extents() const 
{ 
	return extents_; 
}

void engine::add_animated_property(const std::string& name, property::animate_ptr a)
{
	property_manager_.add(name, a);
}

void engine::set_camera(const point& cam) 
{ 
	camera_ = cam;
	clip_camera_to_extents();
}

void engine::set_camera(int x, int y)
{
	camera_.x = x; 
	camera_.y = y;
	clip_camera_to_extents();
}

void engine::clip_camera_to_extents()
{
	if(camera_.y <= extents_.y()) {
		camera_.y = extents_.y();
	}
	if(camera_.x <= extents_.x()) {
		camera_.x = extents_.x();
	}
	if(camera_.y >= extents_.y2() - wm_.height()) {
		camera_.y = extents_.y2() - wm_.height();
	}
	if(camera_.x >= extents_.x2() - wm_.width()) {
		camera_.x = extents_.x2() - wm_.width();
	}
}
