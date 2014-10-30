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
#include <random>

#include "ai_process.hpp"
#include "component.hpp"
#include "engine.hpp"
#include "random.hpp"

namespace process
{
	ai::ai()
		: process(ProcessPriority::ai),
		  should_update_(false),
		  update_turns_(0)
	{
	}

	ai::~ai()
	{
	}

	bool ai::handle_event(const SDL_Event& evt)
	{
		switch(evt.type) {
			case SDL_USEREVENT:
				if(evt.user.code == static_cast<Sint32>(EngineUserEvents::NEW_TURN)) {
					should_update_ = true;
					update_turns_ = static_cast<int>(reinterpret_cast<intptr_t>(evt.user.data1));
				}
			default: break;
		}
		return false;
	}
	
	void ai::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		if(!should_update_) {
			return;
		}
		should_update_ = false;
		for(auto& e : elist) {
			static component_id ai_mask = genmask(Component::POSITION) | genmask(Component::AI) | genmask(Component::INPUT);

			if((e->mask & ai_mask) == ai_mask) {
				auto& pos = e->pos;
				//auto& aip = e->aip;
				auto& inp = e->inp;
			
				// Set default action to nothing
				inp->action = input::Action::none;

				// XXX this should be rate limited a bit, so if the player wanted
				// to do something for 20 turns then we carry out 1 turn/200ms or so
				// then if the player needed to cancel action they could.
				// Fortunately we have a useful time parameter t to use.
				update_turns_ = eng.get_turns() - update_turns_;
				for(int n = 0; n != update_turns_; ++n) {
					// XXX add some logic
					if(generator::get_uniform_int(0,1)) {
						pos->mov.x += generator::get_uniform_int(-1, 1);
					} else {
						pos->mov.y += generator::get_uniform_int(-1, 1);
					}
				}
				inp->action = input::Action::moved;
			}
		}
	}
}
