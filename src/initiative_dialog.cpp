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

namespace gui
{
	initiative::initiative(const rectf& pos, Justify justify)
		: widget(pos, justify)
	{
	}

	void initiative::handle_init()
	{
	}

	void initiative::handle_draw(const point&p, float rotation, float scale) const
	{
		const rect& r = physical_area()+p;

		// XXX draw a filled circle
		static graphics::texture fcircle("images/misc/circle1.png", graphics::TextureFlags::NONE);
		const rect nr(r.top_left(), r.w()/2, r.w()/2s);
		fcircle.blit_ex(nr, 0, point(), graphics::FlipFlags::NONE);

		if(sprites_.empty()) {
			return;
		}

		// XXX future ideas 
		// (1) if two units are at the same time to act,
		// then make them smaller and put one on top the other at the
		// bottom of the intiative bar.
		// (2) Add mouse over or clickable information to units.

		const float early_time = sprites_.front().time_to_act;
		const float late_time  = sprites_.back().time_to_act;

		for(auto& th : sprites_) {
			const float time_scale = (th.time_to_act - early_time) / (late_time - early_time);
			// Calculate the new width/height of the sprite to make it such that it takes up the full height of the 
			// initiative bar, whilst preserving width/height ratio of the sprite.
			const int height = r.h();
			const int width = (th.tex.width() * height) / th.tex.height();
			const rect pos_r(r.x() + static_cast<int>(r.w()*time_scale) - width/2, r.mid_y() - height/2, width, height);

			th.tex.blit_ex(pos_r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
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
		
	}

	void initiative::recalc_dimensions()
	{
	}

	void initiative::handle_window_resize(int w, int h)
	{
	}
}
