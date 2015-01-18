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

#include "button.hpp"
#include "gui_elements.hpp"
#include "wm.hpp"

namespace gui
{
	namespace
	{
		int default_horizontal_padding = 8;
		int default_vertical_padding = 4;
	}

	button::button(std::function<void()> pressed, const rectf& r, Justify justify, widget_ptr child)
		: widget(r, justify),
		  pressed_fn_(pressed),
		  child_(child),
		  is_pressed_(false),
		  is_mouseover_(false),
		  padding_(default_horizontal_padding, default_vertical_padding)
	{
		ASSERT_LOG(r.x() >= -1.0f && r.x() <= 1.0f, "Button X position exceeds parent boundaries.");
		ASSERT_LOG(r.y() >= -1.0f && r.y() <= 1.0f, "Button Y position exceeds parent boundaries.");
		ASSERT_LOG(r.x2() >= -1.0f && r.x2() <= 1.0f, "Button X2 position exceeds parent boundaries.");
		ASSERT_LOG(r.y2() >= -1.0f && r.y2() <= 1.0f, "Button Y2 position exceeds parent boundaries.");
	}

	button::button(std::function<void()> pressed, widget_ptr child, Justify justify)
		: widget(point(), justify),
		  pressed_fn_(pressed),
		  child_(child),
		  is_pressed_(false),
		  is_mouseover_(false),
		  padding_(default_horizontal_padding, default_vertical_padding)
	{
	}

	void button::handle_init()
	{
		normal_tex_ = gui::section::get("buttonLong_brown");
		pressed_tex_ = gui::section::get("buttonLong_brown_pressed");
		mouse_over_tex_ = gui::section::get("mouseover_overlay");
		mouse_over_tex_.set_blend(graphics::BlendMode::ADDITIVE);
		normal_tex_.set_blend(graphics::BlendMode::BLEND);
		pressed_tex_.set_blend(graphics::BlendMode::BLEND);

		if(child_) {
			child_->set_parent(this_pointer());
		}
		recalc_dimensions();
	}

	void button::handle_draw(const point&p, float rotation, float scale) const
	{
		rect r = physical_area()+p;
		const auto& tex = is_pressed_ ? pressed_tex_ : normal_tex_;
		tex.blit_ex(r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);

		if(child_) {
			child_->draw(r.top_left() + padding_, rotation, scale);
		}
		if(is_mouseover_) {
			mouse_over_tex_.blit_ex(r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		}
	}

	bool button::handle_events(SDL_Event* evt, bool claimed)
	{
		SDL_Event evtcopy(*evt);
		normalize_event(&evtcopy, physical_area().top_left());
		if(child_ && child_->process_events(&evtcopy, claimed)) {
			return true;
		}
		auto& wm = graphics::window_manager::get_main_window();
		switch(evt->type) {
			case SDL_MOUSEMOTION: 
				if(in_widget(evt->motion.x, evt->motion.y)) {
					is_mouseover_ = true;
				} else {
					is_mouseover_ = false;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(in_widget(evt->button.x, evt->button.y)) {
					on_press();
					return true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				on_release();
				if(in_widget(evt->button.x, evt->button.y)) {
					return true;
				}
				break;
			default:
				break;
		}
		return false;
	}

	void button::set_padding(int h, int v)
	{
		padding_ = point(h, v);
		recalc_dimensions();
	}

	void button::on_press()
	{
		is_pressed_ = true;
		if(pressed_fn_) {
			pressed_fn_();
		}
		recalc_dimensions();
	}

	void button::on_release()
	{
		if(is_pressed_) {
			// XXX Change texture to normal one
			is_pressed_ = false;
			recalc_dimensions();
		}
	}

	void button::recalc_dimensions()
	{
		if(!is_area_set()) {
			if(child_) {
				set_dim_internal(child_->w() + 2 * padding_.x, child_->h() + 2 * padding_.y);
			} else if(is_pressed_) {
				set_dim_internal(pressed_tex_.width() + 2 * padding_.x, pressed_tex_.height() + 2 * padding_.y);
			} else {
				set_dim_internal(normal_tex_.width() + 2 * padding_.x, normal_tex_.height() + 2 * padding_.y);
			}
		}
	}

	void button::handle_window_resize(int w, int h)
	{
		if(child_) {
			child_->window_resize(w, h);
		}
	}
}
