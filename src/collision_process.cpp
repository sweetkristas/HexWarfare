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

#include "component.hpp"
#include "engine.hpp"
#include "collision_process.hpp"

namespace process
{
	ee_collision::ee_collision()
		: process(ProcessPriority::collision)
	{
	
	}
	
	ee_collision::~ee_collision()
	{
	}

	// XXX Should we split this into a map/entity collision and entity/entity collision
	// classes?
	void ee_collision::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		static component_id collision_mask = genmask(Component::POSITION) | genmask(Component::COLLISION);
		// O(n^2) collision testing is for the pro's :-/
		// XXX Please make quad-tree or kd-tree for O(nlogn)
		for(auto& e1 : elist) {
			if((e1->mask & collision_mask) == collision_mask) {
				auto& e1pos = e1->pos;
				// XXX replace elist_ below with a call to eng.entities_in_area(r)
				// where r = active area for entities on map.
				for(auto& e2 : elist) {
					if(e1 == e2) {
						continue;
					}
					if((e2->mask & collision_mask) == collision_mask) {
						// entity - entity collision
					}
				}
			}
		}
	}

	em_collision::em_collision()
		: process(ProcessPriority::collision)
	{
	}
	
	em_collision::~em_collision()
	{
	}

	void em_collision::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		//static component_id collision_mask = genmask(Component::COLLISION) | genmask(Component::POSITION);
		// O(n^2) collision testing is for the pro's :-/
		// XXX Please make quad-tree or kd-tree for O(nlogn)
		for(auto& e1 : elist) {
			// XXX
		}
	}
}
