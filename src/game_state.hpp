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

#include "geometry.hpp"
#include "hex_logical_fwd.hpp"
#include "message_format.pb.h"
#include "player.hpp"
#include "units_fwd.hpp"
#include "uuid.hpp"

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

		const unit_list& get_entities() const { return units_; }

		void set_map(hex::logical::map_ptr map);
		const hex::logical::map_ptr& get_map() const { return map_; }

		void add_unit(unit_ptr e);
		void remove_unit(unit_ptr e);

		void end_unit_turn();
		float get_initiative_counter() const { return initiative_counter_; }

		// Players are abstract and not entities in this case, since we need special handling.
		void add_player(player_ptr p);
		void remove_player(player_ptr p);
		void replace_player(player_ptr to_be_replaced, player_ptr replacement);

		player_ptr get_current_player() const;
		int get_player_count() const { return players_.size(); }
		player_ptr get_player(const uuid::uuid& n);
		std::vector<player_ptr> get_players();

		bool is_attackable(const unit_ptr& aggressor, const unit_ptr& e) const;

		Update* create_update() const;
		const state& unit_summon(Update* up, unit_ptr e) const;
		const state& unit_move(Update* up, unit_ptr e, const std::vector<point>& path) const;
		const state& unit_attack(Update* up, const unit_ptr& e, const std::vector<unit_ptr>& targets) const;
		const state& end_turn(Update* up) const;

		// Server-side function for validating the received update.
		Update* validate_and_apply(Update* up);
		// Client-side function for processing recived update, checking the reply
		// And making client side stuff happen. (i.e. animated moving -- if we haven't done so already)
		// validating that the update counter is correct. 
		// Adjusting everything if it's a re-sync update.
		void apply(Update* up);

		team_ptr create_team_instance(const std::string& name);
		team_ptr get_team_from_id(const uuid::uuid& id);

		unit_ptr create_unit_instance(const std::string& name, const player_ptr& pid, const point& pos);

		const player_ptr& get_player_by_uuid(const uuid::uuid& id) const;

	private:
		float initiative_counter_;
		mutable int update_counter_;
		hex::logical::map_ptr map_;
		// List of game entities with stats tag. Sorted by intiative.
		unit_list units_;
		std::map<uuid::uuid, player_ptr> players_;
		// Used to synchronise state with the server.
		std::string fail_reason_;
		std::map<uuid::uuid, team_ptr> teams_;

		unit_ptr get_unit_by_uuid(const uuid::uuid& id);
		void set_validation_fail_reason(const std::string& reason);

		void combat(Update* up, Update_Unit* agg_uu, unit_ptr aggressor, unit_ptr target);

		void set_unit_stats(unit_ptr e, const Update_UnitStats& stats);

		bool validate_move(const unit_ptr& u, const ::google::protobuf::RepeatedPtrField<Update_Location>& path);
	};
}
