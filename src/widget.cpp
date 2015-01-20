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

#include "widget.hpp"
#include "wm.hpp"

namespace gui
{
	widget::widget(const rectf& r, Justify justify)
		: area_(r),
		  area_i_(),
		  actual_area_(),
		  xy_is_fp_(true),
		  wh_is_fp_(true),
		  zorder_(0), 
		  rotation_(0), 
		  scale_(1.0f),
		  area_set_(false),
		  just_(justify),
		  parent_(),
		  enabled_(true),
		  background_rect_enabled_(false),
		  background_rect_color_(255,255,255)
	{
		area_set_ = !area_.empty();
		if(area_set_) {
			update_area();
		}
	}

	widget::widget(const point& p, Justify justify)
		: area_(),
		  area_i_(p, 0, 0),
		  actual_area_(),
		  xy_is_fp_(false),
		  wh_is_fp_(false),
		  zorder_(0), 
		  rotation_(0), 
		  scale_(1.0f),
		  area_set_(false),
		  just_(justify),
		  parent_(),
		  enabled_(true),
		  background_rect_enabled_(false),
		  background_rect_color_(255,255,255)
	{
		area_set_ = !area_i_.empty();
		if(area_set_) {
			update_area();
		}
	}

	widget::widget(const rect& r, Justify justify)
		: area_(),
		  area_i_(r),
		  actual_area_(),
		  xy_is_fp_(false),
		  wh_is_fp_(false),
		  zorder_(0), 
		  rotation_(0), 
		  scale_(1.0f),
		  area_set_(false),
		  just_(justify),
		  parent_(),
		  enabled_(true),
		  background_rect_enabled_(false),
		  background_rect_color_(255,255,255)
	{
		area_set_ = !area_i_.empty();
		if(area_set_) {
			update_area();
		}
	}

	widget::~widget()
	{
	}

	bool widget::in_widget(const point& p)
	{
		return geometry::pointInRect(p, actual_area_ * get_scale());
	}

	bool widget::in_widget(int x, int y)
	{
		return geometry::pointInRect(x, y, actual_area_ * get_scale());
	}

	void widget::draw(const rect& r, float rotation, float scale) const
	{
		ASSERT_LOG(!actual_area_.empty(), "No dimensions set.");

		if(background_rect_enabled_) {
			auto& wm = graphics::window_manager::get_main_window();
			SDL_Rect fr = { actual_area_.x(), actual_area_.y(), actual_area_.w(), actual_area_.h() };
			SDL_SetRenderDrawColor(wm.get_renderer(), background_rect_color_.r(), background_rect_color_.g(), background_rect_color_.b(), background_rect_color_.a());
			SDL_RenderFillRect(wm.get_renderer(), &fr);
			SDL_SetRenderDrawColor(wm.get_renderer(), 0, 0, 0, 255);
		}

		handle_draw(physical_area()+r.top_left(), rotation+rotation_, scale*scale_);

		// XXX if not enabled should draw something over it to indicate such.
		if(!enabled_) {
		}
	}

	bool widget::process_events(SDL_Event* evt, bool claimed)
	{
		if(claimed || !enabled_) {
			return claimed;
		}
		return handle_events(evt, claimed);
	}

	void widget::update(const engine& eng, double t)
	{
		handle_update(eng, t);
	}

	void widget::set_justification(Justify j)
	{
		just_ = j;
		update_area();
	}

	void widget::normalize_event(SDL_Event* evt, const point& p)
	{
		switch(evt->type) {
		case SDL_MOUSEMOTION:
			evt->motion.x = evt->motion.x - p.x;
			evt->motion.y = evt->motion.y - p.y;
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			evt->button.x = evt->button.x - p.x;
			evt->button.y = evt->button.y - p.y;
			break;
		default:
			break;
		}
	}

	void widget::update_area()
	{
		const int pw = get_parent_absolute_width();
		const int ph = get_parent_absolute_height();

		int x = area_i_.x();
		int y = area_i_.y();
		int w = area_i_.w();
		int h = area_i_.h();
		if(xy_is_fp_) {
			x = static_cast<int>(area_.x() * pw);
			y = static_cast<int>(area_.y() * ph);
		}
		if(wh_is_fp_) {
			w = static_cast<int>(area_.w() * pw);
			h = static_cast<int>(area_.h() * ph);
		}

		if(just_ & Justify::H_CENTER) {
			x = (pw - w)/2 + x;
		} else if(just_ & Justify::RIGHT) {
			x = pw - w + x;
		}
		if(just_ & Justify::V_CENTER) {
			y = (ph - h)/2 + y;
		} else if(just_ & Justify::BOTTOM) {
			y = ph - h + y;
		}

		actual_area_.set(x, y, w, h);
	}

	void widget::set_loc(int x, int y)
	{
		area_i_.set(x, y, area_i_.w(), area_i_.h());
		xy_is_fp_ = false;
		update_area();
	}

	void widget::set_dim(int w, int h)
	{
		area_i_.set_wh(w, h);
		wh_is_fp_ = false;
		update_area();
	}

	void widget::set_loc(float x, float y)
	{
		area_.set(x, y, area_.w(), area_.h());		
		xy_is_fp_ = true;
		update_area();
	}

	void widget::set_dim(float w, float h)
	{
		area_.set_wh(w, h);
		wh_is_fp_ = true;
		update_area();
	}

	void widget::set_parent(std::weak_ptr<widget> parent)
	{
		parent_ = parent;
		recalc_dimensions();
		update_area();
	}

	int widget::get_parent_absolute_width()
	{
		auto& wm = graphics::window_manager::get_main_window();
		auto owner = parent_.lock();
		if(owner == nullptr) {
			return wm.width();
		}
		return owner->w();
	}

	int widget::get_parent_absolute_height()
	{
		auto& wm = graphics::window_manager::get_main_window();
		auto owner = parent_.lock();
		if(owner == nullptr) {
			return wm.height();
		}
		return owner->h();
	}

	void widget::window_resize(int w, int h)
	{
		handle_window_resize(w, h);
		update_area();
	}
}
