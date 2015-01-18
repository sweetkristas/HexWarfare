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

#include "widget.hpp"

namespace gui
{
	class button : public widget
	{
	public:
		MAKE_FACTORY(button);
	private:
		explicit button(std::function<void()> pressed, const rectf& pos, Justify justify=Justify::TOP_LEFT, widget_ptr child=nullptr);
		explicit button(std::function<void()> pressed, widget_ptr child=nullptr, Justify justify=Justify::TOP_LEFT);
		void handle_init() override;
		void handle_draw(const point&p, float rotation, float scale) const override;
		bool handle_events(SDL_Event* evt, bool claimed) override;
		void recalc_dimensions() override;
		void handle_window_resize(int w, int h) override;
		void on_press();
		void on_release();
		std::function<void()> pressed_fn_;
		widget_ptr child_;
		bool is_pressed_;
		bool is_mouseover_;
		graphics::texture normal_tex_;
		graphics::texture pressed_tex_;
		graphics::texture mouse_over_tex_;

		button() = default;
		button(const button&) = delete;
		void operator=(const button&) = delete;
	};
}
