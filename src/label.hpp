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

#pragma once

#include "color.hpp"
#include "font.hpp"
#include "widget.hpp"

namespace gui
{
	class label : public widget
	{
	public:
		MAKE_FACTORY(label)

		void set_text(const std::string& utf8);
		void set_size(int sz);
		void set_color(const graphics::color& color);
		void set_font(const std::string& font_name);
	private:
		explicit label(const std::string& utf8, const graphics::color& color=graphics::color(), int sz=12, const point& p=point(), Justify j=Justify::TOP_LEFT);
		explicit label(const rectf& pos, Justify justify, const std::string& utf8, const graphics::color& color=graphics::color(), int sz=12);
		void handle_init() override;
		void handle_draw(const rect& r, float rotation, float scale) const override;
		void handle_window_resize(int w, int h);
		void recalc_dimensions() override;
		std::string text_;
		int size_;
		graphics::color color_;
		std::string font_name_;
		font::font_ptr font_;
		graphics::texture label_tex_;

		label(const label&) = delete;
	};
}
