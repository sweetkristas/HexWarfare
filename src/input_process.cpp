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
		static component_id input_mask = component::genmask(component::Component::INPUT);
		static component_id pos_mask = component::genmask(component::Component::POSITION);
		// Clear the input queue of keystrokes for now.
		while(!keys_pressed_.empty()) {
			keys_pressed_.pop();
		}
		if(!mouse_button_events_.empty()) {
			auto button = mouse_button_events_.front(); mouse_button_events_.pop();
			for(auto& e : elist) {
				if((e->mask & pos_mask) == pos_mask && (e->mask & input_mask) == input_mask) {
					auto& pos = e->pos->pos;
					auto& inp = e->inp;
					auto pp = hex::hex_map::get_pixel_pos_from_tile_pos(pos.x, pos.y);
					if(button.button == SDL_BUTTON_LEFT 
						&& button.type == SDL_MOUSEBUTTONUP) {
						if(geometry::pointInRect(point(button.x, button.y), inp->mouse_area + pp)) {
							inp->selected = true;
						} else {
							inp->selected = false;
						}
					}
				}
			}
		}
	}
}
