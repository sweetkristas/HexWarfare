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
#include "font.hpp"
#include "render_process.hpp"

namespace process
{
	namespace
	{
		void draw_position_text(SDL_Renderer* r, int w, int h, const hex::hex_object* tile)
		{
			font::font_ptr fnt = font::get_font("SourceCodePro-Regular.ttf", 12);
			std::stringstream ss1;
			ss1 << "Tile under mouse: " << tile->x() << ", " << tile->y();
			auto surf = font::render_shaded(ss1.str(), fnt, graphics::color(1.0f, 1.0f, 0.5f), graphics::color(0, 0, 0));
			auto tex = SDL_CreateTextureFromSurface(r, surf);
			SDL_Rect dst = {0, h - surf->h, surf->w, surf->h};
			SDL_RenderCopy(r, tex, NULL, &dst);
			SDL_FreeSurface(surf);
			SDL_DestroyTexture(tex);
		}
	}

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
		static component_id inp_mask = genmask(Component::INPUT);
		
		const point& cam = eng.get_camera();
		const float zoom = eng.get_zoom();
		const point screen_centre(eng.get_window().width() / 2, eng.get_window().height() / 2);
		const point& ts = eng.get_tile_size();

		SDL_RenderSetScale(eng.get_renderer(), zoom, zoom);

		hex::hex_map_ptr game_map = nullptr;

		for(auto& e : elist) {
			if((e->mask & sprite_mask) == sprite_mask && (e->mask & inp_mask) == inp_mask && e->inp->selected) {
				auto& pos = e->pos;
				static auto ellipse = graphics::texture("images/misc/ellipse-1.png", graphics::TextureFlags::NONE);
				auto pp = hex::hex_map::get_pixel_pos_from_tile_pos(pos->pos.x, pos->pos.y);
				const int x = pp.x - cam.x + (ts.x - ellipse.width())/2;
				const int y = pp.y - cam.y + ts.y - ellipse.height();
				ellipse.blit(rect(x, y, ellipse.width(), ellipse.height()));
			}

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
					spr->tex.blit(rect(pp.x - cam.x, pp.y - cam.y, ts.x, ts.y));
				}
			} else if((e->mask & map_mask) == map_mask) {
				// XXX It'd probably be better to remove the map as being an entity and just store it as a singleton.
				// Then we can collision detect/draw it from there.
				auto& map = e->map;

				const int screen_width_in_tiles = (eng.get_window().width() + eng.get_tile_size().x - 1) / eng.get_tile_size().x;
				const int screen_height_in_tiles = (eng.get_window().height() + eng.get_tile_size().y - 1) / eng.get_tile_size().y;
				//rect area = rect::from_coordinates(-screen_width_in_tiles / 2 + cam.x, 
				//	-screen_height_in_tiles / 2 + cam.y,
				//	screen_width_in_tiles / 2 + cam.x,
				//	screen_height_in_tiles / 2 + cam.y);

				map->map->draw(cam);

				game_map = map->map; 
			}
		}

		// draw cursor, aligned to hexes. -- should do this better.
		if(game_map) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			x = static_cast<int>(x / eng.get_zoom());
			y = static_cast<int>(y / eng.get_zoom());
			auto tile_pos = game_map->get_tile_from_pixel_pos(x + cam.x, y + cam.y);
			//std::cerr << "XXX: (" << x << "," << y << "), (" << cam.x << "," << cam.y << "), (" << tile_pos->x() << "," << tile_pos->y() << ")\n";
			if(tile_pos) {
				static auto overlay = graphics::texture("images/misc/overlay1.png", graphics::TextureFlags::NONE);
				point p = game_map->get_pixel_pos_from_tile_pos(tile_pos->x(), tile_pos->y());
				overlay.blit(rect(p.x - cam.x, p.y - cam.y, ts.x, ts.y));
				draw_position_text(eng.get_renderer(), eng.get_window().width(), eng.get_window().height(), tile_pos);
			}
		}

		
		SDL_RenderSetScale(eng.get_renderer(), 1.0f, 1.0f);
	}
}
