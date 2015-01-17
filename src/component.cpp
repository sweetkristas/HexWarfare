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

#include "asserts.hpp"
#include "component.hpp"
#include "units.hpp"

namespace component
{
	component_set::component_set(int z) 
		: mask(component_id(0)), 
		  zorder(z),
		  pos(),
		  lifetime(0)
	{
	}

	component_set::component_set(const component_set& cs)
		: mask(cs.mask),
		  zorder(cs.zorder),
		  pos(cs.pos),
		  lifetime(0)
	{
		if(cs.spr != nullptr) {
			spr = cs.spr->clone();
		}
		if(cs.stat != nullptr) {
			stat = cs.stat->clone(cs.stat->get_owner());
		}
		if(cs.inp != nullptr) {
			inp = cs.inp->clone();
		}
		if(cs.gui != nullptr) {
			gui = cs.gui->clone();
		}
	}

	Component get_component_from_string(const std::string& s)
	{
		if(s == "position") {
			return Component::POSITION;
		} else if(s == "sprite") {
			return Component::SPRITE;
		} else if(s == "stats") {
			return Component::STATS;
		} else if(s == "input") {
			return Component::INPUT;
		} else if(s == "lights") {
			return Component::LIGHTS;
		} else if(s == "gui") {
			return Component::GUI;
		} else if(s == "collision") {
			return Component::COLLISION;
		}
		ASSERT_LOG(false, "Unrecognised component string '" << s << "'");
		return static_cast<Component>(0);
	}

	sprite::sprite(const std::string& filename, const rect& area)
		: component(Component::SPRITE),
		  tex("images/" + filename, graphics::TextureFlags::NONE, area)
	{
	}

	sprite::sprite(surface_ptr surf, const rect& area) 
		: component(Component::SPRITE),
		  tex(surf, graphics::TextureFlags::NONE, area)
	{
	}

	sprite::~sprite()
	{
	}

	void sprite::update_texture(surface_ptr surf)
	{
		tex.update(surf);
	}

	lights::lights() 
		: component(Component::LIGHTS)
	{
	}

	lights::~lights()
	{
	}
}

std::ostream& operator<<(std::ostream& os, const component_set_ptr& e)
{
	using namespace component;
	static component_id unit_mask = genmask(Component::STATS) | genmask(Component::POSITION);
	if((e->mask & unit_mask) == unit_mask) {
		os << e->stat;
	} else {
		// XX fill this out more
	}
	return os;
}
