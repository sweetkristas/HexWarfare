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
		init();
	}

	void dialog::handle_draw(const rect& r, float rotation, float scale) const
	{
		if(bg_.is_valid()) {
			bg_.blit_ex(r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		}
		for(auto& w : children_) {
			w->draw(r, rotation, scale);
		}
	}

	void dialog::recalc_dimensions()
	{
		if(!is_area_set() && bg_.is_valid()) {
			set_dim_internal(bg_.width(), bg_.height());
		}
	}

	void dialog::init()
	{
		bg_ = gui::section::get("panel_beige");
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
		w->set_parent(this);
		children_.emplace_back(w);
		std::stable_sort(children_.begin(), children_.end());
	}
	
	void dialog::handle_update(engine& eng, double t)
	{
		// XXX
	}
}
