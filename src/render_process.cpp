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
#include "render_process.hpp"

namespace process
{
	render::render() 
		: process(ProcessPriority::render) 
	{
	}
	
	render::~render() 
	{
	}

	void render::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		static component_id sprite_mask = genmask(Component::POSITION)  | genmask(Component::SPRITE);
		static component_id gui_mask = sprite_mask | genmask(Component::GUI);
		static component_id map_mask = genmask(Component::MAP);
		
		const point& cam = eng.get_camera();
		const point screen_centre(eng.get_window().width() / 2, eng.get_window().height() / 2);
		const point& ts = eng.get_tile_size();

		for(auto& e : elist) {
			if((e->mask & gui_mask) == gui_mask) {
				auto& spr = e->spr;
				auto& pos = e->pos;
				auto& g = e->gui;
				if(spr->tex.is_valid()) {
					spr->tex.blit(rect(pos->pos.x, pos->pos.y));
				}
				for(auto& w : g->widgets) {
					w->draw(rect(0, 0, eng.get_window().width(), eng.get_window().height()), 0.0f, 1.0f);
				}
			}  else if((e->mask & sprite_mask) == sprite_mask) {
				auto& spr = e->spr;
				auto& pos = e->pos;
				if(spr->tex.is_valid()) {
					auto pp = hex::hex_map::get_pixel_pos_from_tile_pos(pos->pos.x, pos->pos.y);
					spr->tex.blit(rect(pp.x - cam.x * ts.x, pp.y - cam.y * ts.y, ts.x, ts.y));
				}
			} else if((e->mask & map_mask) == map_mask) {
				auto& map = e->map;

				const int screen_width_in_tiles = (eng.get_window().width() + eng.get_tile_size().x - 1) / eng.get_tile_size().x;
				const int screen_height_in_tiles = (eng.get_window().height() + eng.get_tile_size().y - 1) / eng.get_tile_size().y;
				//rect area = rect::from_coordinates(-screen_width_in_tiles / 2 + cam.x, 
				//	-screen_height_in_tiles / 2 + cam.y,
				//	screen_width_in_tiles / 2 + cam.x,
				//	screen_height_in_tiles / 2 + cam.y);

				map->map->draw(cam);
			}
		}
	}
}
