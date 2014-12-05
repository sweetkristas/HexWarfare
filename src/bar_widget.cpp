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

#include "bar_widget.hpp"
#include "gui_elements.hpp"
#include "wm.hpp"

namespace gui
{
	bar::bar(const rectf& pos, Justify justify, BarOrientation orientation) 
		: widget(pos, justify),
		  orientation_(orientation)
	{
		if(orientation_ == BarOrientation::HORIZONTAL) {
			endcap_lt_ = section::get("barBlue_horizontalLeft");
			endcap_rb_ = section::get("barBlue_horizontalRight");
			bar_middle_ = section::get("barBlue_horizontalMid");
		} else {
			endcap_lt_ = section::get("barBlue_verticalTop");
			endcap_rb_ = section::get("barBlue_verticalBottom");
			bar_middle_ = section::get("barBlue_verticalMid");
		}
	}

	void bar::handle_init() 
	{
	}

	void bar::handle_draw(const rect& r, float rotation, float scale) const 
	{
		auto& wm = graphics::window_manager::get_main_window();
		int lt_x, lt_y;
		int rb_x, rb_y;
		int lt_w, lt_h;
		int rb_w, rb_h;
		rect mid_r;
		if(orientation_ == BarOrientation::HORIZONTAL) {
			lt_x = r.x();
			lt_y = r.y();
			lt_w = endcap_lt_.width();
			lt_h = r.h();
			
			rb_x = r.x2() - endcap_rb_.width();
			rb_y = r.y();
			rb_w = endcap_rb_.width();
			rb_h = r.h();

			mid_r = rect(lt_x+lt_w-1, lt_y, rb_x-lt_x+lt_w-1, lt_h);
		} else {
			lt_x = r.x();
			lt_y = r.y();
			lt_w = r.w();
			lt_h = endcap_lt_.height();

			rb_x = r.x();
			rb_y = r.y2() - endcap_rb_.height();
			rb_w = r.h();
			rb_h = endcap_rb_.height();

			mid_r = rect(lt_x, lt_y+lt_h-1, lt_w, rb_y-lt_y+lt_h-1);
		}
		bar_middle_.blit_ex(mid_r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		endcap_lt_.blit_ex(rect(lt_x, lt_y, lt_w, lt_h) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		endcap_rb_.blit_ex(rect(rb_x, rb_y, rb_w, rb_h) * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
	}

	void bar::recalc_dimensions() 
	{
	}
}
