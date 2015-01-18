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

#include "label.hpp"

namespace gui
{
	label::label(const rectf& pos, Justify justify, const std::string& utf8, const graphics::color& color, int sz)
		: widget(pos, justify), 
		  text_(utf8),
		  size_(sz),
		  color_(color)
	{
	}

	label::label(const std::string& utf8, const graphics::color& color, int sz, Justify j)
		: widget(j),
		  text_(utf8),
		  size_(sz),
		  color_(color)
	{
	}

	void label::set_text(const std::string& utf8)
	{
		text_ = utf8;
		init();
	}

	void label::set_size(int sz)
	{
		size_ = sz;
		init();
	}

	void label::set_color(const graphics::color& color)
	{
		color_ = color;
		init();
	}

	void label::set_font(const std::string& font_name)
	{
		font_name_ = font_name;
		init();
	}

	void label::handle_draw(const point&p, float rotation, float scale) const
	{
		rect r = physical_area()+p;
		if(label_tex_.is_valid()) {
			label_tex_.blit_ex(r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		}
	}

	void label::recalc_dimensions()
	{
		if(label_tex_.is_valid() && !is_area_set()) {
			set_dim_internal(label_tex_.width(), label_tex_.height());
		}
	}

	void label::handle_init()
	{
		font_ = font::get_font(font_name_.empty() ? font::get_default_font_name() : font_name_, size_);
		auto s = std::make_shared<graphics::surface>(font::render(text_, font_, color_));
		label_tex_ = graphics::texture(s, graphics::TextureFlags::NO_CACHE);
		recalc_dimensions();
	}

	void label::handle_window_resize(int w, int h)
	{
	}
}
