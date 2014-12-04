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
	class grid : public widget
	{
	public:
		MAKE_FACTORY(grid);
		
		grid& add_item(widget_ptr c);
	private:
		explicit grid(const rectf& r, Justify justify, int ncols);
		void recalc_dimensions() override;
		void handle_init() override;
		void handle_draw(const rect& r, float rotation, float scale) const override;
		bool handle_events(SDL_Event* evt, bool claimed) override;

		int ncols_;
		std::vector<std::vector<widget_ptr>> children_;
	};
}
