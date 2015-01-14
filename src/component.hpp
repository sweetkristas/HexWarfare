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
#include <string>
#include <vector>

#include "SDL.h"

#include "color.hpp"
#include "creature.hpp"
#include "geometry.hpp"
#include "hex_map.hpp"
#include "hex_pathfinding.hpp"
#include "node.hpp"
#include "player.hpp"
#include "texture.hpp"
#include "uuid.hpp"
#include "widget.hpp"

#define CLONE(cname) std::shared_ptr<cname> clone() { return std::shared_ptr<cname>(new cname(*this)); }

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
		GUI,
		// tag only values must go at end.
		CREATURE,
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
		position(const point& p) : component(Component::POSITION), pos(p), gs_pos(p) {}
		CLONE(position)
		point pos;
		point gs_pos;
	};

	struct sprite : public component
	{
		sprite() : component(Component::SPRITE) {}
		sprite(surface_ptr surf, const rect& area=rect());
		sprite(const std::string& filename, const rect& area=rect());
		~sprite();
		CLONE(sprite)
		void update_texture(surface_ptr surf);
		graphics::texture tex;
	};

	struct stats : public component
	{
		stats() : component(Component::STATS), health(1), attack(0), armour(0), move(1.0f), range(1) {}
		CLONE(stats)
		// N.B. If things are added or removed here, this needs to be reflected in the message_format.proto file.
		// specifically game::Update::UnitStats
		int health;
		int attack;
		int armour;
		float move;
		float initiative;
		std::string name;
		int range;
		creature::const_creature_ptr unit;
	};

	struct input : public component
	{
		input() : component(Component::INPUT), clear_selection(false), selected(false), is_attack_target(false) {}
		CLONE(input)
		rect mouse_area;
		bool clear_selection;
		bool selected;
		bool is_attack_target;
		hex::result_list possible_moves;
		hex::hex_graph_ptr graph;
		std::vector<point> arrow_path;
		std::vector<point> tile_path;
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
		CLONE(lights)
		// XXX These should in some sort of quadtree like structure.
		std::vector<point_light> light_list;
		graphics::texture tex;
	};

	struct gui_component : public component
	{
		gui_component() : component(Component::GUI) {}
		CLONE(gui_component)
		std::vector<gui::widget_ptr> widgets;
	};

	struct component_set
	{
		component_set(int z = 0, uuid::uuid u=uuid::generate());
		component_set(const component_set&, player_weak_ptr new_owner);
		std::shared_ptr<component_set> clone(player_weak_ptr new_owner) { return std::shared_ptr<component_set>(new component_set(*this, new_owner)); }
		uuid::uuid entity_id;
		component_id mask;
		int zorder;
		// XXX Since pos is frequently accessed, it'd be better if it was just point pos; in the declaration.
		std::shared_ptr<position> pos;
		std::shared_ptr<sprite> spr;
		std::shared_ptr<stats> stat;
		std::shared_ptr<input> inp;
		std::shared_ptr<gui_component> gui;
		player_weak_ptr owner;
		// XXX add direct access to team uuid and maybe owner uuid
		component_set(const component_set&) = delete;
		void operator=(const component_set&) = delete;
	};
	
	inline bool operator<(const component_set_ptr& lhs, const component_set_ptr& rhs)
	{
		return lhs->zorder == rhs->zorder ? lhs.get() < rhs.get() : lhs->zorder < rhs->zorder;
	}

	inline bool initiative_compare(const component_set_ptr& lhs, const component_set_ptr& rhs)
	{
		static component_id stats_mask = genmask(Component::STATS);
		ASSERT_LOG((lhs->mask & stats_mask) == stats_mask, "initiative_compare: LHS of comparison does not have STATS component.");
		ASSERT_LOG((rhs->mask & stats_mask) == stats_mask, "initiative_compare: RHS of comparison does not have STATS component.");
		return lhs->stat->initiative < rhs->stat->initiative;
	}
}

std::ostream& operator<<(std::ostream& os, const component_set_ptr& e);

typedef std::shared_ptr<component::component_set> component_set_ptr;
typedef std::weak_ptr<component::component_set> component_set_weak_ptr;
