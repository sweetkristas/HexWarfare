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

	grid::grid(int ncols, Justify justify)
		: widget(point(), justify),
		  ncols_(ncols),
		  children_()
	{
	}

	grid& grid::add_item(widget_ptr c)
	{
		if(children_.size() == 0 || static_cast<int>(children_.back().size()) >= ncols_) {
			std::vector<widget_ptr> e;
			children_.emplace_back(e);
		}
		children_.back().emplace_back(c);
		c->set_parent(this_pointer());
		ASSERT_LOG(!c->has_fixed_location(), "Giving the grid widget children with fixed locations isn't supported.");
		recalc_dimensions();
		return *this;
	}

	void grid::recalc_dimensions()
	{
		if(!is_area_set()) {
			int max_w = 0;
			int max_h = 0;
			for(auto& row : children_) {
				int h = 0;
				int w = 0;
				for(auto& c : row) {
					w += c->w();
					if(c->h() > h) {
						h = c->h();
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

	void grid::handle_init()
	{
		recalc_dimensions();
	}

	void grid::handle_draw(const point&p, float rotation, float scale) const
	{
		int last_h = 0;
		for(auto& row : children_) {
			int h = 0;
			int w = 0;
			for(auto& c : row) {
				c->draw(physical_area().top_left()+p+point(w,last_h), rotation, scale);
				if(c->h() > h) {
					h = c->h();
				}
				w += c->w();
			}
			last_h = h;
		}
	}

	void grid::handle_window_resize(int w, int h)
	{
		for(auto& row : children_) {
			for(auto& c : row) {
				c->window_resize(w, h);
			}
		}
	}

	bool grid::handle_events(SDL_Event* evt, bool claimed)
	{
		int last_h = 0;
		for(auto& row : children_) {
			int h = 0;
			int w = 0;
			for(auto& c : row) {
				// The way this code works is disappointing -- in the extreme.
				SDL_Event evtcopy(*evt);
				normalize_event(&evtcopy, physical_area().top_left() + point(w, last_h));
				claimed = c->process_events(&evtcopy, claimed);
				if(claimed) {
					return claimed;
				}
				if(c->h() > h) {
					h = c->h();
				}
				w += c->w();
			}
			last_h = h;
		}
		return claimed;
	}
}
