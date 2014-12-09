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

#include "engine.hpp"
#include "node.hpp"

namespace creature
{
	class creature;
	typedef std::shared_ptr<creature> creature_ptr;
	typedef std::shared_ptr<const creature> const_creature_ptr;

	enum class MovementType
	{
		NORMAL,
	};

	class creature : public std::enable_shared_from_this<creature>
	{
	public:
		creature(const node& n);
		component_set_ptr create_instance(engine& eng, player_weak_ptr owner, const point& pos);

		int get_initiative() const { return initiative_; }
		float get_movement() const { return movement_; }
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
		// attack type (magic, physical, type of magic, type of physical, etc)
		// has ranged attack
		// what items might be carried.
		// lights
		std::string ai_name_;
		std::string sprite_name_;
		rect sprite_area_;
		component_id component_mask_;
		creature();
	};

	void loader(const node& n);

	// XXX should the spawn function automatically add the entity to the engine?
	component_set_ptr spawn(engine& eng, player_weak_ptr owner, const std::string& type, const point& pos);
}
