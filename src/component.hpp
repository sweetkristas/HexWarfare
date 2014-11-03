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

#include <bitset>
#include <memory>
#include <string>
#include <vector>

#include "SDL.h"

#include "color.hpp"
#include "creature.hpp"
#include "geometry.hpp"
#include "hex_map.hpp"
#include "node.hpp"
#include "player.hpp"
#include "texture.hpp"
#include "widget.hpp"

typedef std::bitset<64> component_id;

namespace component
{
	// XXX Todo thing of a cleaner way of doing this with bitsets.
	enum class Component
	{
		POSITION,
		SPRITE,
		STATS,
		INPUT,
		LIGHTS,
		MAP,
		GUI,
		// tag only values must go at end.
		COLLISION,
		MAX_COMPONENTS,
	};
	static_assert(static_cast<int>(Component::MAX_COMPONENTS) <= 64, "Maximum number of components must be less than 64. If you need more consider a vector<bool> solution.");

	Component get_component_from_string(const std::string& s);

	inline component_id operator<<(int value, const Component& rhs) 
	{
		return component_id(value << static_cast<unsigned long long>(rhs));
	}
	inline component_id genmask(const Component& lhs)
	{
		return 1 << lhs;
	}

	class component
	{
	public:
		explicit component(Component id) : id_(component_id(static_cast<unsigned long long>(id))) {}
		virtual ~component() {}
		component_id id() const { return id_; }
	private:
		const component_id id_;
	};

	typedef std::shared_ptr<component> component_ptr;

	struct position : public component
	{
		position() : component(Component::POSITION) {}
		position(const point& p) : component(Component::POSITION), pos(p) {}
		point pos;
		point mov;
	};

	struct sprite : public component
	{
		sprite() : component(Component::SPRITE) {}
		sprite(surface_ptr surf, const rect& area=rect());
		sprite(const std::string& filename, const rect& area=rect());
		~sprite();
		void update_texture(surface_ptr surf);
		graphics::texture tex;
	};

	struct stats : public component
	{
		stats() : component(Component::STATS), health(1), attack(0), armour(0) {}
		int health;
		int attack;
		int armour;
		std::string name;
		creature::const_creature_ptr unit;
	};

	struct input : public component
	{
		input() : component(Component::INPUT), selected(false) {}
		rect mouse_area;
		bool selected;
	};

	struct point_light
	{
		point_light(int xx, int yy, const graphics::color& cc, float att)
			: x(xx), y(yy), color(cc), attenuation(att) 
		{}
		int x;
		int y;
		graphics::color color;
		float attenuation;
	};

	struct lights : public component
	{
		lights();
		~lights();
		// XXX These should in some sort of quadtree like structure.
		std::vector<point_light> light_list;
		graphics::texture tex;
	};

	struct mapgrid : public component
	{
		mapgrid();
		hex::hex_map_ptr map;
	};

	struct gui_component : public component
	{
		gui_component() : component(Component::GUI) {}
		std::vector<gui::widget_ptr> widgets;
	};

	struct component_set
	{
		component_set(int z = 0);
		size_t entity_id;
		component_id mask;
		int zorder;
		std::shared_ptr<position> pos;
		std::shared_ptr<sprite> spr;
		std::shared_ptr<stats> stat;
		std::shared_ptr<input> inp;
		std::shared_ptr<mapgrid> map;
		std::shared_ptr<gui_component> gui;
		player_weak_ptr owner;
	};
	typedef std::shared_ptr<component_set> component_set_ptr;
	typedef std::weak_ptr<component_set> component_set_weak_ptr;
	
	inline bool operator<(const component_set_ptr& lhs, const component_set_ptr& rhs)
	{
		return lhs->zorder == rhs->zorder ? lhs.get() < rhs.get() : lhs->zorder < rhs->zorder;
	}
}
