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

#include "bar_widget.hpp"

namespace gui
{
	class initiative : public widget
	{
	public:
		MAKE_FACTORY(initiative);
	private:
		explicit initiative(const rectf& pos, Justify justify);
		void handle_init() override;
		void handle_draw(const point&p, float rotation, float scale) const override;
		void handle_update(const engine& eng, double t) override;
		void handle_window_resize(int w, int h);
		void recalc_dimensions() override;

		struct texture_holder
		{
			texture_holder(const graphics::texture& t, float toa) : tex(t), time_to_act(toa) {}
			graphics::texture tex;
			float time_to_act;
		};
		std::vector<texture_holder> sprites_;
	};
}
