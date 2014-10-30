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

#include <iomanip>
#include <sstream>

#include "component.hpp"
#include "datetime.hpp"
#include "engine.hpp"
#include "font.hpp"
#include "gui_process.hpp"

namespace process
{
	gui::gui() 
		: process(ProcessPriority::gui)
	{
	}

	gui::~gui()
	{
	}

	void gui::update(engine& eng, double t, const entity_list& elist)
	{
		using namespace component;
		static component_id gui_mask
			= genmask(Component::GUI)
			| genmask(Component::STATS)
			| genmask(Component::POSITION)
			| genmask(Component::SPRITE);
		
		for(auto& e : elist) {
			if((e->mask & gui_mask) == gui_mask) {
				auto& stats = e->stat;
				auto& pos = e->pos;
				auto& spr = e->spr;
				auto fnt = font::get_font("SourceCodePro-Regular.ttf", 20);
				// XXX we should color code some of these
				// i.e. as health gets lower it goes from green to red, to grey dead/unconscious
				std::stringstream ss;
				ss  << "Turn: " << std::setw(4) << eng.get_turns()
					<< " | "
					<< "Health: " << stats->health
					<< " | "
					<< datetime(eng.get_turns()).printable()
					;
				auto surf = font::render_shaded(ss.str(), fnt, graphics::color(255,255,255), graphics::color(0,0,0));
				// surface gets free'd by update_texture, so we need to get height (and width if needed) before we call it.
				// gui tagged entities get absolute pixel positioning for *free*.
				pos->pos.x = 0;
				pos->pos.y = eng.get_window().height() - surf->h;
				spr->update_texture(std::make_shared<graphics::surface>(surf));
			}
		}
	}
}
