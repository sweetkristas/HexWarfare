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

#include "image_widget.hpp"

namespace gui
{
	image::image(graphics::texture t, const rectf& r, Justify justify)
		: widget(r, justify),
		  tex_(t)
	{
	}

	image::image(graphics::texture t, Justify j)
		: widget(j),
		  tex_(t)
	{
	}

	void image::recalc_dimensions()
	{
		if(!is_area_set()) {
			set_dim_internal(tex_.width(), tex_.height());
		}
	}

	void image::handle_init()
	{
		recalc_dimensions();
	}

	void image::handle_draw(const point&p, float rotation, float scale) const
	{
		rect r = physical_area()+p;
		tex_.blit_ex(r, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
	}
}
