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
#include "engine.hpp"
#include "node_utils.hpp"
#include "profile_timer.hpp"

namespace 
{
	const double mouse_event_scale_factor = 65535.0;
}

double get_mouse_scale_factor()
{
	return mouse_event_scale_factor;
}

engine::engine(graphics::window_manager& wm)
	: state_(EngineState::PLAY),
	  turns_(1),
	  wm_(wm),
	  entity_quads_(0, rect(0,0,100,100)),
	  particles_(wm.get_renderer()),
	  current_player_(0)
{
}

engine::~engine()
{
}

component_set_ptr engine::add_entity(component_set_ptr e)
{
	entity_list_.emplace_back(e);
	std::stable_sort(entity_list_.begin(), entity_list_.end());
	return e;
}

void engine::remove_entity(component_set_ptr e1)
{
	entity_list_.erase(std::remove_if(entity_list_.begin(), entity_list_.end(), [&e1](component_set_ptr e2) {
		return e1 == e2; 
	}), entity_list_.end());
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

void engine::add_player(player_ptr p)
{
	players_.emplace_back(p);
}

void engine::remove_player(player_ptr p)
{
	auto it = std::find(players_.begin(), players_.end(), p);
	ASSERT_LOG(it != players_.end(), "Attempted to remove player " << p->name() << " failed, player doesn't exist.");
	players_.erase(it);
}

void engine::replace_player(player_ptr to_be_replaced, player_ptr replacement)
{
	auto it = std::find(players_.begin(), players_.end(), to_be_replaced);
	ASSERT_LOG(it != players_.end(), "Attempted to remove player " << to_be_replaced->name() << " failed, player doesn't exist.");
	*it = replacement;
}

void engine::translate_mouse_coords(SDL_Event* evt)
{
	// transform the absolute mouse co-ordinates to a window-size independent quantity.
	if(evt->type == SDL_MOUSEMOTION) {
		//evt->motion.x = static_cast<Sint32>((evt->motion.x * mouse_event_scale_factor) / wm_.width());
		//evt->motion.y = static_cast<Sint32>((evt->motion.y * mouse_event_scale_factor) / wm_.height());
	} else {
		//evt->button.x = static_cast<Sint32>((evt->button.x * mouse_event_scale_factor) / wm_.width());
		//evt->button.y = static_cast<Sint32>((evt->button.y * mouse_event_scale_factor) / wm_.height());
	}
}

void engine::process_events()
{
	SDL_Event evt;
	while(SDL_PollEvent(&evt)) {
		bool claimed = false;
		switch(evt.type) {
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
				translate_mouse_coords(&evt);
				break;
			case SDL_MOUSEWHEEL:
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
				if(evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					set_state(EngineState::QUIT);
					return;
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

void engine::populate_quadtree()
{
	entity_quads_.clear();
	
	// only add entities to the quadtree that meet are collidable, but not maps
	static component_id collision_mask 
		= (1 << component::Component::POSITION)
		| (1 << component::Component::SPRITE)
		| (1 << component::Component::COLLISION);
	static component_id collision_map_mask = collision_mask | (1 << component::Component::MAP);

	for(auto& e : entity_list_) {
		if((e->mask & collision_map_mask) == collision_mask) {
			auto& pos = e->pos->pos;
			auto& spr = e->spr;
			entity_quads_.insert(e, rect(pos.x, pos.y, tile_size_.x, tile_size_.y));
		}
	}
}

entity_list engine::entities_in_area(const rect& r)
{
	entity_list res;
	entity_quads_.get_collidable(res, r);
	return res;
}

bool engine::update(double time)
{
	process_events();
	if(state_ == EngineState::PAUSE || state_ == EngineState::QUIT) {
		return state_ == EngineState::PAUSE ? true : false;
	}

	populate_quadtree();
	for(auto& p : process_list_) {
		p->update(*this, time, entity_list_);
	}

	// Player move hack

	particles_.update(static_cast<float>(time));
	particles_.draw();
	return true;
}

void engine::inc_turns(int cnt)
{ 
	// N.B. The event holds the number turn number before we incremented. 
	// So we can run things like the AI for the correct number of turns skipped.
	// XXX On second thoughts I don't like this at all, we should skip
	// turns at a rate 1/200ms or so I think. Gives player time to cancel
	// if attacked etc.
	SDL_Event user_event;
	user_event.type = SDL_USEREVENT;
	user_event.user.code = static_cast<Sint32>(EngineUserEvents::NEW_TURN);
	user_event.user.data1 = reinterpret_cast<void*>(turns_);
	user_event.user.data2 = nullptr;
	SDL_PushEvent(&user_event);

	turns_ += cnt;
}


const player_ptr& engine::get_current_player() const
{
	ASSERT_LOG(current_player_ < players_.size(), "current_player is out of bounds: " << current_player_ << " >= " << players_.size());
	return players_[current_player_];
}
