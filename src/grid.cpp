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
		recalc_dimensions();
		return *this;
	}

	void grid::recalc_dimensions()
	{
		int max_w = 0;
		int max_h = 0;
		int last_h = 0;
		for(auto& row : children_) {
			int h = 0;
			int w = 0;
			for(auto& c : row) {
				c->set_loc(w, last_h);
				w += c->w();
				if(c->h() > h) {
					h = c->h();
				}
			}
			last_h = h;
			max_h += h;
			if(w > max_w) {
				max_w = w;
			}
			int nw = 0;
		}
		set_dim(max_w, max_h);
	}

	void grid::handle_init()
	{
		recalc_dimensions();
	}

	void grid::handle_draw(const rect& r, float rotation, float scale) const
	{
		for(auto& row : children_) {
			for(auto& c : row) {
				c->draw(r, rotation, scale);
			}
		}
	}

	void grid::handle_window_resize(int w, int h)
	{
		for(auto& row : children_) {
			for(auto& c : row) {
				c->window_resize(w, h);
			}
		}
		recalc_dimensions();
	}

	bool grid::handle_events(SDL_Event* evt, bool claimed)
	{
		SDL_Event evtcopy(*evt);
		normalize_event(&evtcopy, physical_area().top_left());

		for(auto& row : children_) {
			for(auto& c : row) {
				claimed = c->process_events(&evtcopy, claimed);
				if(claimed) {
					return claimed;
				}
			}
		}
		return claimed;
	}
}
