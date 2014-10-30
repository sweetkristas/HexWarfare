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
#include "input_process.hpp"

namespace process
{
	input::input()
		: process(ProcessPriority::input)
	{
	}

	input::~input()
	{
	}

	bool input::handle_event(const SDL_Event& evt)
	{
		if(evt.type == SDL_KEYDOWN) {
			keys_pressed_.push(evt.key.keysym.scancode);
			return true;
		}
		return false;
	}

	void input::update(engine& eng, double t, const entity_list& elist)
	{
		static component_id input_mask 
			= component::genmask(component::Component::POSITION) 
			| component::genmask(component::Component::INPUT)
			| component::genmask(component::Component::PLAYER);
		for(auto& e : elist) {
			if((e->mask & input_mask) == input_mask) {
				auto& inp = e->inp;
				auto& pos = e->pos;
				inp->action = component::input::Action::none;
				if(!keys_pressed_.empty()) {
					auto key = keys_pressed_.front();
					keys_pressed_.pop();
					if(key == SDL_SCANCODE_LEFT) {
						pos->mov.x -= 1;
						inp->action = component::input::Action::moved;
					} else if(key == SDL_SCANCODE_RIGHT) {
						pos->mov.x += 1;
						inp->action = component::input::Action::moved;
					} else if(key == SDL_SCANCODE_UP) {
						pos->mov.y -= 1;
						inp->action = component::input::Action::moved;
					} else if(key == SDL_SCANCODE_DOWN) {
						pos->mov.y += 1;
						inp->action = component::input::Action::moved;
					} else if(key == SDL_SCANCODE_PERIOD) {
						inp->action = component::input::Action::pass;
					}
				}
			}
		}
	}
}
