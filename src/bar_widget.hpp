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
	enum class BarOrientation
	{
		HORIZONTAL,
		VERTICAL,
	};

	class bar : public widget
	{
	public:
		MAKE_FACTORY(bar);
	private:
		explicit bar(const rectf& pos, Justify justify, BarOrientation orientation=BarOrientation::HORIZONTAL);
		void handle_init() override;
		void handle_draw(const rect& r, float rotation, float scale) const override;
		void recalc_dimensions() override;

		graphics::texture endcap_lt_;
		graphics::texture endcap_rb_;
		graphics::texture bar_middle_;

		BarOrientation orientation_;
	};
}
