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

#include "component.hpp"
#include "engine.hpp"
#include "initiative_dialog.hpp"

namespace gui
{
	initiative::initiative(const rectf& pos, Justify justify)
		: widget(pos, justify)
	{
	}

	void initiative::handle_init()
	{
		if(!bar_) {
			bar_ = bar::create(rectf(0.0f, 0.0f, 1.0f, 0.2f), gui::Justify::LEFT | gui::Justify::V_CENTER, gui::BarOrientation::HORIZONTAL);
		}
	}

	void initiative::handle_draw(const rect& r, float rotation, float scale) const
	{
		if(bar_) {
			bar_->draw(r, rotation, scale);
		}

		for(auto& th : sprites_) {
			// XXX calculate correct position.
			rect pos_r;
			th.tex.blit_ex(pos_r * scale, rotation, r.mid() * scale, graphics::FlipFlags::NONE);
		}
	}

	void initiative::handle_update(engine& eng, double t)
	{
		const component_id stat_mask = component::genmask(component::Component::STATS) | component::genmask(component::Component::SPRITE);
		sprites_.clear();

		for(auto& e : eng.get_entities()) {
			if((e->mask & stat_mask) == stat_mask) {
				auto ini = e->stat->initiative;
				auto spr = e->spr->tex;
				sprites_.emplace_back(spr, ini);
			}
		}
		std::stable_sort(sprites_.begin(), sprites_.end(), [](const texture_holder& lhs, const texture_holder& rhs) {
			return lhs.time_to_act < rhs.time_to_act;
		});
	}

	void initiative::recalc_dimensions()
	{
	}
}
