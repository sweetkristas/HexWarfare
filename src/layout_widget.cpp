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

#include <boost/range/adaptor/reversed.hpp>

#include "asserts.hpp"
#include "layout_widget.hpp"

namespace gui
{
	layout::layout(const std::vector<widget_ptr>& children, LayoutType lt, const rectf& pos, Justify justify)
		: widget(pos, justify),
		  layout_type_(lt)
	{
		for(auto& c : children) {
			children_.emplace(c);
		}
	}

	layout::layout(const std::vector<widget_ptr>* children, LayoutType lt,Justify justify)
		: widget(point(), justify),
		  layout_type_(lt)
	{
		if(children != nullptr) {
			for(auto& c : *children) {
				children_.emplace(c);
			}
		}
	}

	void layout::add_child(widget_ptr w)
	{
		children_.emplace(w);
	}

	void layout::handle_init()
	{
	}

	void layout::handle_draw(const point&p, float rotation, float scale) const
	{
		for(auto& w : children_) {
			// This will work for absolutely positioned elements, not for relative ones.
			w->draw(physical_area().top_left()+p, rotation, scale);
		}
	}

	bool layout::handle_events(SDL_Event* evt, bool claimed)
	{
		for(auto& w : boost::adaptors::reverse(children_)) {
			SDL_Event evtcpy(*evt);
			if((claimed = w->process_events(&evtcpy, claimed)) == true) {
				return claimed;
			}
		}
		return claimed;
	}

	void layout::recalc_dimensions()
	{
		int lx = 0;
		int ly = 0;
		int lw = 0;
		int lh = 0;
		if(layout_type_ == LayoutType::LT_RELATIVE) {
			/*for(auto& w : children_) {
				ASSERT_LOG(w->w() < fixed_width_, "width of child widget is greater than width of layout widget");
				if(lx + w->w() > fixed_width_) {
					ly += lh;
					lh = 0;
				}
				lh = std::max(lh, w->h());
				w->setLoc(lx, ly);
				lx += w->w();
				lw = std::max(lw, lx);
			}*/
			ASSERT_LOG(false, "add code for relative layout.");
		} else if(layout_type_ == LayoutType::LT_ABSOLUTE) {
			// do nothing
			for(auto& w : children_) {
				lw = std::max(lw, w->w());
				lh = std::max(lh, w->h());
			}
		} else {
			ASSERT_LOG(false, "Incorrect layout style");
		}
		if(!is_area_set()) {
			set_dim_internal(lw, lh);
		}
	}

	void layout::handle_window_resize(int w, int h)
	{
	}


}
