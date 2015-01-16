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

#include "grid.hpp"

namespace gui
{
	grid::grid(const rectf& r, Justify justify, int ncols)
		: widget(r, justify),
		  ncols_(ncols),
		  children_()
	{
	}

	grid& grid::add_item(widget_ptr c)
	{
		if(children_.size() == 0 || children_.back().size() >= static_cast<size_t>(ncols_)) {
			std::vector<widget_ptr> e;
			children_.emplace_back(e);
		}
		children_.back().emplace_back(c);
		c->set_parent(get_pointer());
		init();
		return *this;
	}

	void grid::recalc_dimensions()
	{
		if(!is_area_set()) {
			float ow = 0, oh = 0;
			if(children_.empty()) {
				set_dim_internal(1,1);
				return;
			}
			for(auto& r : children_) {
				float lw = 0;
				float lh = 0;
				for(auto& c : r) {
					lw += c->get_area().w();
					if(c->get_area().h() > lh) {
						lh = c->get_area().h();
					}
				}
				if(lw > ow) {
					ow = lw;
				}
				oh += lh;
			}
//#error fixme
			//set_dim_internal();
		}
	}

	void grid::handle_init()
	{
		if(!is_area_set()) {
			int max_w = 0;
			int max_h = 0;
			for(auto& row : children_) {
				int h = 0;
				int w = 0;
				for(auto& c : row) {
					w += c->get_area().w();
					if(c->get_area().h() > h) {
						h = c->get_area().h();
					}
				}
				max_h += h;
				if(w > max_w) {
					max_w = w;
				}
			}
			set_dim_internal(max_w, max_h);
		}
	}

	void grid::handle_draw(const rect& r, float rotation, float scale) const
	{
		for(auto& row : children_) {
			for(auto& c : row) {
				c->draw(r, rotation, scale);
			}
		}
	}

	bool grid::handle_events(SDL_Event* evt, bool claimed)
	{
		return claimed;
	}
}
