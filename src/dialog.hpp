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
	enum class BackgroundSections
	{
		CORNER_TL,
		CORNER_TR,
		CORNER_BL,
		CORNER_BR,
		SIDE_LEFT,
		SIDE_RIGHT,
		SIDE_TOP,
		SIDE_BOTTOM,
		CENTER,
		MAX
	};

	class dialog : public widget
	{
	public:
		MAKE_FACTORY(dialog);
		void close();
		void add_widget(widget_ptr w);
	private:
		explicit dialog(const rectf& pos, Justify justify);
		virtual bool handle_events(SDL_Event* evt, bool claimed) override;
		void handle_draw(const rect& r, float rotation, float scale) const override;
		void handle_update(engine& eng, double t) override;
		void handle_init() override;
		void recalc_dimensions() override;
		std::vector<graphics::texture> bg_;
		std::vector<widget_ptr> children_;
		bool is_open_;
	};
}
