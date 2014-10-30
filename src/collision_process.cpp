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
#include "map.hpp"
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
		static component_id collision_map_mask = collision_mask | genmask(Component::MAP);
		// O(n^2) collision testing is for the pro's :-/
		// XXX Please make quad-tree or kd-tree for O(nlogn)
		for(auto& e1 : elist) {
			if((e1->mask & collision_map_mask) == collision_mask) {
				auto& e1pos = e1->pos;
				// XXX replace elist_ below with a call to eng.entities_in_area(r)
				// where r = active area for entities on map.
				for(auto& e2 : elist) {
					if(e1 == e2) {
						continue;
					}
					if((e2->mask & collision_mask) == collision_mask) {
						// entity - entity collision
						auto& e2pos = e2->pos;
						if(e1pos->pos + e1pos->mov == e2pos->pos) {
							e1pos->mov.clear();
						}						
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
		static component_id collision_mask = genmask(Component::COLLISION) | genmask(Component::POSITION);
		static component_id  map_mask      = genmask(Component::COLLISION) | genmask(Component::MAP);
		// O(n^2) collision testing is for the pro's :-/
		// XXX Please make quad-tree or kd-tree for O(nlogn)
		for(auto& e1 : elist) {
			if((e1->mask & map_mask) == map_mask) {
				//auto& e1map = e1->get()->map;
				for(auto& e2 : elist) {
					if(e1 == e2) {
						continue;
					}
					if((e2->mask & (collision_mask | genmask(Component::MAP))) == collision_mask) {
						//auto& e2pos = e2->pos;
						// entity - map collision
						//if(!is_passable(e1map->map[e2pos->p.y][e2pos->p.x])) {
						//	e2pos->p = e2pos->last_p;
						//}
						//for(auto& exitp : e1map->exits) {
						//	if(e2pos->p == exitp) {
						//		if(e2->is_player()) {
						//			// XXX do some exit logic, quitting the game for now
						//			eng.set_state(EngineState::QUIT);
						//		} else {
						//			// don't let the non-player controlled entities leave via exit
						//			e2pos->p = e2pos->last_p;
						//		}
						//	}
						//}
					} 
				}
			}
		}
	}
}
