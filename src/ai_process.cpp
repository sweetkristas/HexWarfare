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
		}
	}
}
