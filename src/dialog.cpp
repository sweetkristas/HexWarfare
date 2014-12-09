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

#include <algorithm>

#include "dialog.hpp"
#include "engine.hpp"
#include "gui_elements.hpp"

namespace gui 
{
	dialog::dialog(const rectf& pos, Justify justify)
		: widget(pos, justify),
		  is_open_(false)
	{
	}

	void dialog::handle_draw(const rect& r, float rotation, float scale) const
	{
		for(auto& tex : bg_) {
			ASSERT_LOG(tex.is_valid(), "Texture isn't valid.");
		}
		
		auto& tl = bg_[static_cast<int>(BackgroundSections::CORNER_TL)];
		tl.blit_ex(rect(r.x(), r.y(), tl.width(), tl.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& tr = bg_[static_cast<int>(BackgroundSections::CORNER_TR)];
		tr.blit_ex(rect(r.x2() - tr.width(), r.y(), tr.width(), tr.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& bl = bg_[static_cast<int>(BackgroundSections::CORNER_BL)];
		bl.blit_ex(rect(r.x(), r.y2() - bl.height(), bl.width(), bl.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& br = bg_[static_cast<int>(BackgroundSections::CORNER_BR)];
		br.blit_ex(rect(r.x2() - br.width(), r.y2() - br.height(), br.width(), br.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& cc = bg_[static_cast<int>(BackgroundSections::CENTER)];
		cc.blit_ex(rect(r.x()+tl.width(), r.y()+tl.height(), r.w() - br.width() - tl.width(), r.h() - br.height() - tl.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& sl = bg_[static_cast<int>(BackgroundSections::SIDE_LEFT)];
		sl.blit_ex(rect(r.x(), r.y()+tl.height(), tl.width(), r.h()-bl.height()-tl.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& sr = bg_[static_cast<int>(BackgroundSections::SIDE_RIGHT)];
		sr.blit_ex(rect(r.x2() - sr.width(), r.y() + tr.height(), sr.width(), r.h()-tr.height()-br.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& st = bg_[static_cast<int>(BackgroundSections::SIDE_TOP)];
		st.blit_ex(rect(r.x() + tl.width(), r.y(), r.w() - tl.width() - tr.width(), st.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		auto& sb = bg_[static_cast<int>(BackgroundSections::SIDE_BOTTOM)];
		sb.blit_ex(rect(r.x() + br.width(), r.y2() - sb.height(), r.w() - bl.width() - br.width(), sb.height()) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);

		for(auto& w : children_) {
			w->draw(r, rotation, scale);
		}
	}

	void dialog::recalc_dimensions()
	{
	}

	void dialog::handle_init()
	{
		// N.B. The ordering here is important. Technically I could use a std::pair<BackgroundSections,graphics::texture> 
		// but that seems overkill.
		bg_.emplace_back(gui::section::get("panel_beige_tl_corner"));
		bg_.emplace_back(gui::section::get("panel_beige_tr_corner"));
		bg_.emplace_back(gui::section::get("panel_beige_bl_corner"));
		bg_.emplace_back(gui::section::get("panel_beige_br_corner"));
		bg_.emplace_back(gui::section::get("panel_beige_left_border"));
		bg_.emplace_back(gui::section::get("panel_beige_right_border"));
		bg_.emplace_back(gui::section::get("panel_beige_top_border"));
		bg_.emplace_back(gui::section::get("panel_beige_bottom_border"));
		bg_.emplace_back(gui::section::get("panel_beige_center"));
		recalc_dimensions();
	}

	void dialog::close()
	{
	}

	bool dialog::handle_events(SDL_Event* evt, bool claimed)
	{
		for(auto& w : children_) {
			w->process_events(evt, claimed);
		}
		return claimed;
	}

	void dialog::add_widget(widget_ptr w)
	{
		w->set_parent(shared_from_this());
		children_.emplace_back(w);
		std::stable_sort(children_.begin(), children_.end());
	}
	
	void dialog::handle_update(engine& eng, double t)
	{
		// XXX
	}
}
