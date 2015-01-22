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

#include <string>

#include "creature_fwd.hpp"
#include "geometry.hpp"
#include "player.hpp"
#include "units_fwd.hpp"
#include "uuid.hpp"

namespace game
{
	// Game state defintion of a unit.
	// Related to component::position and component::stat.
	class unit
	{
	public:
		unit(const std::string& name, const creature::const_creature_ptr& cp, const player_ptr& owner, const uuid::uuid& id=uuid::generate());

		const point& get_position() const { return pos_; }
		void set_position(const point& p) { pos_ = p; }
		void set_position(int x, int y) { pos_.x = x; pos_.y = y; }

		const uuid::uuid& get_uuid() const { return uuid_; }

		int get_health() const { return health_; }
		void set_health(int h) { health_ = h; }
		int get_attack() const { return attack_; }
		void set_attack(int a) { attack_ = a; }
		int get_armour() const { return armour_; }
		void set_armour(int a) { armour_ = a; }
		float get_move() const { return move_; }
		void set_move(float m) { move_ = m; }
		float get_initiative() const { return initiative_; }
		void set_initiative(float i) { initiative_ = i; }
		const std::string& get_name() const { return name_; }
		void set_name(const std::string& n) { name_ = n; }
		int get_range() const { return range_; }
		void set_range(int r) { range_ = r; }
		float get_critical_strike() const { return critical_strike_; }
		void set_critical_strike(float cs) { critical_strike_ = cs; }
		int get_attacks_this_turn() const { return attacks_this_turn_; }
		void set_attacks_this_turn(int att) { attacks_this_turn_ = att; }
		void dec_attacks_this_turn() { attacks_this_turn_ -= 1; }

		player_ptr get_owner() const;
		void set_owner(const player_ptr& new_owner) { owner_ = new_owner; }
		
		// Called at the start of unit's turn to do start of turn type activities.
		void start_turn(Update_UnitStats* uus);
		// Called at the end of a unit's turn to do end of turn activities.
		// such as resetting movement counts, initiative, etc.
		void complete_turn(Update_UnitStats* uus);

		const creature::const_creature_ptr& get_type() const { return type_; }

		unit_ptr clone(const player_ptr& new_owner);
	private:
		// Position of the unit in units consistent with the map defintion.
		point pos_;
		// Units unique identifier
		uuid::uuid uuid_;
		// player that owns this unit.
		player_weak_ptr owner_;

		// N.B. If things are added or removed here, this needs to be reflected in the message_format.proto file.
		// specifically game::Update::UnitStats
		// Also the game_state.cpp needs to be updated. Mostly the state::set_entity_stats() function.
		int health_;
		int attack_;
		int armour_;
		float move_;
		float initiative_;
		std::string name_;
		int range_;
		float critical_strike_;
		int attacks_this_turn_;
		const creature::const_creature_ptr type_;
	};

	inline bool initiative_compare(const unit_ptr& lhs, const unit_ptr& rhs)
	{
		return lhs->get_initiative() < rhs->get_initiative();
	}

	std::ostream& operator<<(std::ostream& os, const unit_ptr& u);

	inline bool operator==(const unit_ptr& lhs, const unit_ptr& rhs)
	{
		return lhs->get_uuid() == rhs->get_uuid();
	}

	inline bool operator!=(const unit_ptr& lhs, const unit_ptr& rhs)
	{
		return !operator==(lhs, rhs);
	}
}
