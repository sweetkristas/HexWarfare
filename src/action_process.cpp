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

#include "action_process.hpp"
#include "component.hpp"
#include "engine.hpp"

namespace process
{
	action::action()
		: process(ProcessPriority::action)
	{
	}

	action::~action()
	{
	}

	void action::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		static component_id mask = genmask(Component::INPUT) | genmask(Component::POSITION);
		for(auto& e : elist) {
			if((e->mask & mask) == mask) {
				auto& inp = e->inp;
				auto& pos = e->pos;
			}
		}
	}
}
