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
#include "initiative_dialog.hpp"
#include "units.hpp"
#include "wm.hpp"

namespace gui
{
	namespace 
	{
		point get_coords_for_intiative(float value, int radius)
		{
			const float angle = /*static_cast<float>(M_PI / 2.0) -*/ ((value - std::floor(value / 100.0f) * 100.0f)) / 100.0f * static_cast<float>(M_PI * 2.0) - static_cast<float>(M_PI / 2.0);
			int x2 = static_cast<int>(static_cast<float>(radius) * std::cos(angle));
			int y2 = static_cast<int>(static_cast<float>(radius) * std::sin(angle));
			return point(x2, y2);
		}
	}

	initiative::initiative(const rectf& pos, Justify justify)
		: widget(pos, justify),
		  sprites_(),
		  current_initiative_(0)
	{
	}

	initiative::initiative(const rect& pos, Justify justify)
		: widget(pos, justify),
		  sprites_(),
		  current_initiative_(0)
	{
	}

	void initiative::handle_init()
	{
		recalc_dimensions();
	}

	void initiative::handle_draw(const rect& r, float rotation, float scale) const
	{
		// XXX draw a filled circle
		static graphics::texture fcircle("images/misc/circle1.png", graphics::TextureFlags::NONE);
		const rect nr(r.top_left(), r.w(), r.w());
		fcircle.blit_ex(nr, 0, point(), graphics::FlipFlags::NONE);

		// draw a line from center to position based on current initiative.
		// convert current initiative value to an angle.
		auto& wm = graphics::window_manager::get_main_window();
		point p = get_coords_for_intiative(current_initiative_, r.w()/2);
		SDL_RenderDrawLine(wm.get_renderer(), r.mid_x(), r.mid_y(), p.x + r.mid_x(), p.y + r.mid_y());

		if(sprites_.empty()) {
			return;
		}

		// XXX future ideas 
		// (1) if two units are at the same time to act,
		// then make them smaller and put one on top the other at the
		// bottom of the intiative bar.
		// (2) Add mouse over or clickable information to units.

		//const float early_time = sprites_.front().time_to_act;
		//const float late_time  = sprites_.back().time_to_act;

		for(auto& th : sprites_) {
			//const float time_scale = (th.time_to_act - early_time) / (late_time - early_time);
			// Calculate the new width/height of the sprite to make it such that it takes up the full height of the 
			// initiative bar, whilst preserving width/height ratio of the sprite.
			//const int height = r.h();
			//const int width = (th.tex.width() * height) / th.tex.height();
			//const rect pos_r(r.x() + static_cast<int>(r.w()*time_scale) - width/2, r.mid_y() - height/2, width, height);

			//th.tex.blit_ex(pos_r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);

			// XXX fix this to display the sprite so that the midpoint of rect(p,th.tex.width()/2,th.tex.height()/2) is at p.
			p = get_coords_for_intiative(th.time_to_act, r.w()/2) + r.mid();
			// calculate quadrant.
			const int w1 = th.tex.width()/2;
			const int h1 = th.tex.height()/2;
			const int x1 = p.x >= r.mid_x() ? p.x - w1/2 : p.x + w1/2;
			const int y1 = p.y >= r.mid_y() ? p.y - h1/2 : p.y + h1/2;
			const rect r1(x1, y1, w1, h1);
			th.tex.blit_ex(r1 * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		}
	}

	void initiative::handle_update(const engine& eng, double t)
	{
		sprites_.clear();

		for(auto& u : eng.get_game_state().get_entities()) {
			auto e = eng.get_entity_for_unit_uuid(u->get_uuid());
			auto ini = e->stat->get_initiative();
			auto spr = e->spr->tex;
			sprites_.emplace_back(spr, ini);
		}
		current_initiative_ = eng.get_game_state().get_initiative_counter();
	}

	void initiative::recalc_dimensions()
	{
		if(!is_area_set()) {
			ASSERT_LOG(false, "fixme");
		}
	}

	void initiative::handle_window_resize(int w, int h)
	{
	}
}
