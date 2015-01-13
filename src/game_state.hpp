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

#include "engine_fwd.hpp"
#include "geometry.hpp"
#include "hex_logical_fwd.hpp"
#include "message_format.pb.h"
#include "player.hpp"

namespace game
{
	// Contains the current game state.
	// Logical representation of the game map
	// Locations and stats for units.
	class state
	{
	public:
		state();
		state(const state&);
		~state();

		const entity_list& get_entities() const { return entities_; }

		void set_map(hex::logical::map_ptr map);
		const hex::logical::map_ptr& get_map() const { return map_; }

		void add_entity(component_set_ptr e);
		void remove_entity(component_set_ptr e);

		void end_unit_turn();
		float get_initiative_counter() const { return initiative_counter_; }

		// Players are abstract and not entities in this case, since we need special handling.
		void add_player(player_ptr p);
		void remove_player(player_ptr p);
		void replace_player(player_ptr to_be_replaced, player_ptr replacement);

		player_weak_ptr get_current_player() const;
		int get_player_count() const { return players_.size(); }
		player_ptr get_player(const uuid::uuid& n);
		player_ptr get_player_by_id(int id);

		bool is_attackable(const component_set_ptr& aggressor, const component_set_ptr& e) const;

		Update* create_update() const;
		const state& unit_summon(Update* up, component_set_ptr e) const;
		const state& unit_move(Update* up, component_set_ptr e, const std::vector<point>& path) const;
		const state& unit_attack(Update* up, const component_set_ptr& e, const std::vector<component_set_ptr>& targets) const;
		const state& end_turn(Update* up) const;

		// Server-side function for validating the received update.
		Update* validate_and_apply(Update* up);
		// Client-side function for processing recived update, checking the reply
		// And making client side stuff happen. (i.e. animated moving -- if we haven't done so already)
		// validating that the update counter is correct. 
		// Adjusting everything if it's a re-sync update.
		void apply(Update* up);

	private:
		float initiative_counter_;
		mutable int update_counter_;
		hex::logical::map_ptr map_;
		// List of game entities with stats tag. Sorted by intiative.
		entity_list entities_;
		std::map<uuid::uuid, player_ptr> players_;
		// Used to synchronise state with the server.
		std::string fail_reason_;

		component_set_ptr get_entity_by_uuid(const uuid::uuid& id);
		void set_validation_fail_reason(const std::string& reason);

		void combat(Update* up, component_set_ptr aggressor, component_set_ptr target);

		void set_entity_stats(component_set_ptr e, const Update_UnitStats& stats);

		bool validate_move(component_set_ptr e, const ::google::protobuf::RepeatedPtrField<Update_Location>& path);
	};
}
