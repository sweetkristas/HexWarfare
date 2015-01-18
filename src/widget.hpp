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

#include <memory>
#include <functional>

#include "SDL.h"

#include "color.hpp"
#include "engine_fwd.hpp"
#include "geometry.hpp"
#include "texture.hpp"

#define MAKE_FACTORY(classname)																\
	template<typename... T>																	\
	static std::shared_ptr<classname> create(T&& ... all) {									\
		auto bptr = std::shared_ptr<classname>(new classname(std::forward<T>(all)...));		\
		bptr->init();																		\
		return bptr;																		\
	}

namespace gui
{
	enum class Justify {
		LEFT			= 1, 
		H_CENTER		= 2,
		RIGHT			= 4,
		TOP				= 8,
		V_CENTER		= 16,
		BOTTOM			= 32,

		TOP_LEFT		= 9,
		BOTTOM_RIGHT	= 36,
		CENTER			= 18,

		TOP_RIGHT		= 12,
		BOTTOM_LEFT		= 33,

		TOP_CENTER		= 10,
		BOTTOM_CENTER	= 34,

		CENTER_LEFT		= 17,
		CENTER_RIGHT	= 20,
	};

	inline Justify operator|(Justify lhs, Justify rhs) {
		return static_cast<Justify>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}
	inline bool operator&(Justify lhs, Justify rhs) {
		return (static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs);
	}

	// XXX Add a std::vector<widget_ptr> children_;
	// Then add visitor pattern stuff using a callback for each child to visit every node.

	// XXX A better(?) with widgets is they listen for parent size adjusted or window size adjusted messages.
	// Then recalculate those when things finish. Would be easier to deal with.
	class widget : public std::enable_shared_from_this<widget>
	{
	public:
		explicit widget(const rectf& r, Justify justify);
		explicit widget(Justify justify);
		virtual ~widget();
		void draw(const point&p, float rotation, float scale) const;
		bool process_events(SDL_Event* evt, bool claimed);
		void update(const engine& eng, double t);
		void normalize_event(SDL_Event* evt, const point& p);

		void set_area(const rectf& area);
		const rectf& get_area() const { return area_; }
		bool is_area_set() { return area_set_; }

		void set_loc(int x, int y);
		void set_dim(int w, int h);

		int w() const { return actual_area_.w(); } 
		int h() const { return actual_area_.h(); }
		int x() const { return actual_area_.x(); }
		int y() const { return actual_area_.y(); }
		const rect& physical_area() const { return actual_area_; }

		int get_zorder() const { return zorder_; }
		void set_zorder(int z) { zorder_ = z; }

		float get_rotation() const { return rotation_; }
		void set_rotation(float rotation) { rotation_ = rotation; }

		float get_scale() const { return scale_; }
		void set_scale(float scale) { scale_ = scale; }

		void set_justification(Justify j);

		void set_parent(std::weak_ptr<widget> parent);

		std::shared_ptr<widget> this_pointer() { return shared_from_this(); }

		void enable(bool e = true) { enabled_ = e; }
		bool is_enabled() const { return enabled_; }

		void init() { handle_init(); }

		void enable_background_rect(bool en=true) { background_rect_enabled_ = en; }
		void set_background_rect_color(const graphics::color& c) { background_rect_color_ = c; }

		bool has_fixed_location() const { return has_fixed_location_; }
		bool has_fixed_dimensions() const { return has_fixed_dimensions_; }

		// Called when the window is resized to recalculate the position we should draw at.
		void window_resize(int w, int h);
	protected:
		bool in_widget(const point& p);
		bool in_widget(int x, int y);
		void set_loc_internal(int x, int y);
		void set_dim_internal(int w, int h);
		int get_parent_absolute_width();
		int get_parent_absolute_height();
	private:
		virtual void handle_update(const engine& eng, double t) {}
		virtual void handle_draw(const point&p, float rotation, float scale) const {}
		virtual bool handle_events(SDL_Event* evt, bool claimed) { return claimed; }
		virtual void handle_window_resize(int w, int h) {}
		void update_area();
		virtual void recalc_dimensions() = 0;
		virtual void handle_init() = 0;
		// The elements of area should be defined on the interval (0,1)
		// representing the fraction of the parent.
		rectf area_;
		// Actual area is the area of the widget when you take into account the window/parent dimensions.
		rect actual_area_;
		// If the location have been directly set to fixed co-ordinates, we set this flag.
		bool has_fixed_location_;
		// If the dimensions have been directly set to fixed values, we set this flag
		bool has_fixed_dimensions_;
		int zorder_;
		float rotation_;
		float scale_;
		bool area_set_;
		Justify just_;
		std::weak_ptr<widget> parent_;
		bool enabled_;
		bool background_rect_enabled_;
		graphics::color background_rect_color_;

		widget() = default;
		widget(const widget&) = delete;
		void operator=(const widget&) = delete;
	};

	typedef std::shared_ptr<widget> widget_ptr;

	inline bool operator<(const widget& lhs, const widget& rhs) { return lhs.get_zorder() < rhs.get_zorder(); }
	inline bool operator<(const widget_ptr& lhs, const widget_ptr& rhs) { return *lhs < *rhs; }
}
