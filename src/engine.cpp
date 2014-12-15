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


engine::engine(graphics::window_manager& wm)
	: state_(EngineState::PLAY),
	  turns_(1),
	  camera_scale_(2),
	  wm_(wm),
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
	if((e->mask & component::genmask(component::Component::STATS)) == component::genmask(component::Component::STATS)) {
		entities_initiative_order_.emplace_back(e);
		std::stable_sort(entities_initiative_order_.begin(), entities_initiative_order_.end(), component::initiative_compare);
	}
	return e;
}

void engine::remove_entity(component_set_ptr e1)
{
	entity_list_.erase(std::remove_if(entity_list_.begin(), entity_list_.end(), [&e1](component_set_ptr e2) {
		return e1 == e2; 
	}), entity_list_.end());
	entities_initiative_order_.erase(std::remove_if(entities_initiative_order_.begin(), entities_initiative_order_.end(), [&e1](component_set_ptr e2) {
		return e1 == e2; 
	}), entities_initiative_order_.end());
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

player_ptr engine::get_player(int n)
{
	ASSERT_LOG(n < players_.size(), "Requested player index outside of bounds. " << n << " >= " << players_.size());
	return players_[n];
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
					if(camera_.y <= extents_.y()) {
						camera_.y = extents_.y();
					}
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_A) {
					camera_.x -= tile_size_.x;
					if(camera_.x <= extents_.x()) {
						camera_.x = extents_.x();
					}
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_S) {
					camera_.y += tile_size_.y;
					if(camera_.y >= extents_.y2() - wm_.height()) {
						camera_.y = extents_.y2() - wm_.height();
					}
				} else if(evt.key.keysym.scancode == SDL_SCANCODE_D) {
					camera_.x += tile_size_.x;
					if(camera_.x >= extents_.x2() - wm_.width()) {
						camera_.x = extents_.x2() - wm_.width();
					}
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

bool engine::update(double time)
{
	process_events();
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
	return true;
}

const player_ptr& engine::get_current_player() const
{
	ASSERT_LOG(current_player_ < players_.size(), "current_player is out of bounds: " << current_player_ << " >= " << players_.size());
	return players_[current_player_];
}

// Does end of turn processing. Like incrementing to the next player.
void engine::end_turn()
{	
	if(entities_initiative_order_.size() > 0) {
		auto e = entities_initiative_order_.front();
		e->stat->initiative += 100.0f/e->stat->unit->get_initiative();
		std::stable_sort(entities_initiative_order_.begin(), entities_initiative_order_.end(), component::initiative_compare);
		initiative_counter_ = entities_initiative_order_.front()->stat->initiative;
	}

	std::cerr << "Initative list:";
	for(auto& e : entities_initiative_order_) {
		std::cerr << "   " << e->stat->name << ":" << e->stat->initiative;
	}
	std::cerr << "\n";
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
