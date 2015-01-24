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

#include <map>

#include "creature.hpp"
#include "game_state.hpp"
#include "random.hpp"
#include "units.hpp"

namespace creature
{
	creature::creature(const node& n) 
		: initiative_(5), 
		  movement_(5.0f), 
		  movement_type_(MovementType::NORMAL) ,
		  range_(1.0f),
		  // default critical strike chance is 5%
		  critical_strike_(0.05f),
		  max_units_attackable_(1),
		  attacks_per_turn_(1)
	{
		ASSERT_LOG(n.is_map(), "Creature definitions must be maps");
		ASSERT_LOG(n.has_key("name"), "Must supply a 'name' attribute for the creature.");
		name_ = n["name"].as_string();
				
		ASSERT_LOG(n.has_key("stats"), "'stats component found must supply a 'stats' attribute for the creature " << name_);
		const node& stats = n["stats"];
		ASSERT_LOG(stats.has_key("health"), "Must supply a 'health' attribute for the creature " << name_);
		if(stats["health"].is_int()) {
			health_min_ = health_max_ = static_cast<int>(stats["health"].as_int());
		} else if(stats["health"].is_list()) {
			ASSERT_LOG(stats["health"].num_elements() == 2, "'health' attribute for creature wasn't a list of two integers " << name_);
			health_min_ = static_cast<int>(stats["health"][0].as_int());
			health_max_ = static_cast<int>(stats["health"][1].as_int());
			if(health_min_ > health_max_) {
				std::swap(health_min_, health_max_);
			}
		}
		ASSERT_LOG(health_min_ > 0 && health_max_ > 0, "Health min/max must be greater than zero: " << health_min_ << " : " << health_max_ << " for creature " << name_);

		ASSERT_LOG(stats.has_key("attack"), "Must supply a 'attack' attribute for the creature " << name_);
		if(stats["attack"].is_int()) {
			attack_min_ = attack_max_ = static_cast<int>(stats["attack"].as_int());
		} else if(stats["attack"].is_list()) {
			ASSERT_LOG(stats["attack"].num_elements() == 2, "'attack' attribute for creature wasn't a list of two integers. " << name_);
			attack_min_ = static_cast<int>(stats["attack"][0].as_int());
			attack_max_ = static_cast<int>(stats["attack"][1].as_int());
			if(attack_min_ > attack_max_) {
				std::swap(attack_min_, attack_max_);
			}
		}
		ASSERT_LOG(attack_min_ > 0 && attack_max_ > 0, "Attack min/max must be greater than zero: " << attack_min_ << " : " << attack_max_ << " for creature " << name_);
					
		armour_ = stats.has_key("armour") ? stats["armour"].as_int32() : 0;

		movement_ = stats["movement"].as_float(5.0f);

		initiative_ = stats["initiative"].as_int32(5);
		ASSERT_LOG(initiative_ >= 1 && initiative_ <= 100, "initiative value not in valid range: 1 <= " << initiative_ << " <= 100");

		if(stats.has_key("movement_type")) {
			const std::string& mt = stats["movement_type"].as_string();
			if(mt == "normal") {
				movement_type_ = MovementType::NORMAL;
			} // XXX
		}

		if(stats.has_key("range")) {
			range_ = stats["range"].as_float();
		}

		max_units_attackable_ = std::min(10, std::max(0, stats["max_units_attackable"].as_int32(1)));
		attacks_per_turn_ = std::min(10, std::max(0, stats["attacks_per_turn"].as_int32(1)));
		critical_strike_ = std::min(1.0f, std::max(0.0f, stats["critical_strike"].as_float(0.05f)));

		ASSERT_LOG(n.has_key("animations"), "No 'animations' attribute found for creature " << name_);
		for(auto& a : n["animations"].as_map()) {
			ASSERT_LOG(a.second.is_map() && a.first.is_string(), "Animations should be 'string'/animation info(map) pairs");

			ASSERT_LOG(a.second["image"].is_string(), "image must be string.");
			std::string img_name = a.second["image"].as_string();
			ASSERT_LOG(a.second["area"].is_list() && a.second["area"].num_elements() == 4, "'area' attribute for creature " << name_ << " must be list of four integers (x1,y1,x2,y2). " << a.second["area"].type_as_string());
			rect area = rect::from_coordinates(static_cast<int>(a.second["area"][0].as_int()),
					static_cast<int>(a.second["area"][1].as_int()),
					static_cast<int>(a.second["area"][2].as_int()),
					static_cast<int>(a.second["area"][3].as_int()));

			animations_[a.first.as_string()] = AnimationInfo(img_name, area);
		}
	}

	const creature::AnimationInfo& creature::get_animation_info(const std::string& name) const
	{
		auto it = animations_.find(name);
		ASSERT_LOG(it != animations_.end(), "No animation found with name '" << name << "'");
		return it->second;
	}

	game::unit_ptr creature::create_instance(const game::state& gs, const player_ptr& owner, const point& pos)
	{
		auto u = std::make_shared<game::unit>(name_, shared_from_this(), owner);
		u->set_health(generator::get_uniform_int(health_min_, health_max_));
		u->set_attack(generator::get_uniform_int(attack_min_, attack_max_));
		u->set_armour(armour_);
		u->set_range(static_cast<int>(range_));
		u->set_move(movement_);
		u->set_critical_strike(critical_strike_);
		u->set_attacks_this_turn(attacks_per_turn_);
		u->set_initiative(100.0f/static_cast<float>(initiative_) + gs.get_initiative_counter());
		u->set_position(pos);
		return u;
	}

	/*component_set_ptr creature::create_instance(const game::state& gs, player_weak_ptr owner, const point& pos) {
		component_set_ptr res = std::make_shared<component::component_set>(80);
		using namespace component;
		// XXX fix zorder here.
		if((component_mask_ & genmask(Component::STATS)) == genmask(Component::STATS)) {
			res->stat = std::make_shared<stats>();
			res->stat->health = generator::get_uniform_int(health_min_, health_max_);
			res->stat->attack = generator::get_uniform_int(attack_min_, attack_max_);
			res->stat->armour = armour_;
			res->stat->name = name_;
			res->stat->range = static_cast<int>(range_);
			res->stat->move = static_cast<float>(movement_);
			res->stat->critical_strike = critical_strike_;
			res->stat->attacks_this_turn = attacks_per_turn_;
			res->stat->initiative = 100.0f/static_cast<float>(initiative_) + gs.get_initiative_counter();
			res->stat->unit = shared_from_this();
		}
		if((component_mask_ & genmask(Component::POSITION)) == genmask(Component::POSITION)) {
			res->pos.pos = pos;
			res->pos.gs_pos = pos;
		}
		if((component_mask_ & genmask(Component::SPRITE)) == genmask(Component::SPRITE)) {
			// The whole sprite mess needs fixed, since it doesn't take an area.
			res->spr = std::make_shared<sprite>(sprite_name_, sprite_area_);
		}
		if((component_mask_ & genmask(Component::LIGHTS)) == genmask(Component::LIGHTS)) {
			// XXX?
		}
		if((component_mask_ & genmask(Component::INPUT)) == genmask(Component::INPUT)) {
			res->inp = std::make_shared<input>();
			res->inp->mouse_area = sprite_area_;
			res->inp->selected = false;
		}
		res->mask = component_mask_;
		res->owner = owner;
		return res;
	}*/

	namespace
	{
		typedef std::map<std::string, creature_ptr> creature_cache;
		creature_cache& get_creature_cache()
		{
			static creature_cache res;
			return res;
		}
	}

	void loader(const node& n)
	{
		for(auto& cr : n.as_map()) {
			get_creature_cache()[cr.first.as_string()] = std::make_shared<creature>(cr.second);
		}
	}

	/*component_set_ptr spawn(const game::state& gs, player_weak_ptr owner, const std::string& type, const point& pos)
	{
		auto it = get_creature_cache().find(type);
		ASSERT_LOG(it != get_creature_cache().end(), "Couldn't find a definition for creature of type '" << type << "' in the cache.");
		return it->second->create_instance(gs, owner, pos);
	}*/

	game::unit_ptr spawn(const game::state& gs, const std::string& type, const player_ptr& owner, const point& pos)
	{
		auto it = get_creature_cache().find(type);
		ASSERT_LOG(it != get_creature_cache().end(), "Couldn't find a definition for creature of type '" << type << "' in the cache.");
		return it->second->create_instance(gs, owner, pos);
	}
}
