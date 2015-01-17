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

#include <map>

#include "creature_fwd.hpp"
#include "game_state.hpp"
#include "node.hpp"
#include "units_fwd.hpp"

namespace creature
{
	enum class MovementType
	{
		NORMAL,
	};

	class creature : public std::enable_shared_from_this<creature>
	{
	public:
		creature(const node& n);
		game::unit_ptr create_instance(const game::state& gs, const player_ptr& owner, const point& pos);

		int get_initiative() const { return initiative_; }
		float get_movement() const { return movement_; }

		int get_max_units_attackable() const { return max_units_attackable_; }
		int get_attacks_per_turn() const { return attacks_per_turn_; }

		struct AnimationInfo {
			AnimationInfo() : image_(), area_() {}
			AnimationInfo(const std::string& image, const rect& area) : image_(image), area_(area) {}
			std::string image_;
			rect area_;
			// XXX Add more fields here as nescessary.
		};
		const AnimationInfo& get_animation_info(const std::string& name) const;
	private:
		// Displayable name
		std::string name_;
		int health_min_;
		int health_max_;
		int attack_min_;
		int attack_max_;
		int armour_;
		int initiative_;
		float movement_;
		MovementType movement_type_;
		// default attack range
		float range_;
		// Percentage chance of the attack being a critical strike.
		float critical_strike_;
		// attack type (magic, physical, type of magic, type of physical, etc)
		// has ranged attack
		// what items might be carried.
		//! Number of units that can be attacked at once using the default attack
		int max_units_attackable_;
		//! Numer of attacks per turn for default attack
		int attacks_per_turn_;

		std::map<std::string, AnimationInfo> animations_;
		creature();
	};

	void loader(const node& n);

	game::unit_ptr spawn(const game::state& gs, const std::string& type, const player_ptr& owner, const point& pos);
}
