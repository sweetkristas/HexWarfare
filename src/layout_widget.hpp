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

#include <set>

#include "widget.hpp"

namespace gui
{
	enum class LayoutType {
		LT_ABSOLUTE,
		LT_RELATIVE,
	};

	class layout : public widget
	{
	public:
		MAKE_FACTORY(layout);

		void add_child(widget_ptr w);
	private:
		explicit layout(const std::vector<widget_ptr>& children, LayoutType lt, const rectf& pos, Justify justify=Justify::TOP_LEFT);
		explicit layout(const std::vector<widget_ptr>* children=nullptr, LayoutType lt=LayoutType::LT_ABSOLUTE,Justify justify=Justify::TOP_LEFT);
		void handle_init() override;
		void handle_draw(const rect& r, float rotation, float scale) const override;
		bool handle_events(SDL_Event* evt, bool claimed) override;
		void recalc_dimensions() override;
		void handle_window_resize(int w, int h) override;

		LayoutType layout_type_;
		
		typedef std::set<widget_ptr> WidgetList;
		WidgetList children_;

	};
}
