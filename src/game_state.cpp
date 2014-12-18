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
#include "game_state.hpp"

namespace game
{
	state::state()
		: initiative_counter_(0.0f),
		  update_counter_(0)
	{
	}

	state::~state()
	{
	}

	void state::set_map(hex::logical::map_ptr map)
	{
		map_ = map;
	}

	void state::add_entity(component_set_ptr e)
	{
		entities_.emplace_back(e);
		std::stable_sort(entities_.begin(), entities_.end(), component::initiative_compare);
	}

	void state::remove_entity(component_set_ptr e1)
	{
		entities_.erase(std::remove_if(entities_.begin(), entities_.end(), [&e1](component_set_ptr e2) {
			return e1 == e2; 
		}), entities_.end());
	}

	void state::end_unit_turn()
	{
		if(entities_.size() > 0) {
			auto e = entities_.front();
			e->stat->initiative += 100.0f/e->stat->unit->get_initiative();
			std::stable_sort(entities_.begin(), entities_.end(), component::initiative_compare);
			initiative_counter_ = entities_.front()->stat->initiative;
		}

		std::cerr << "Initative list:";
		for(auto& e : entities_) {
			std::cerr << "   " << e->stat->name << ":" << e->stat->initiative;
		}
		std::cerr << "\n";
	}

	void state::add_player(player_ptr p)
	{
		players_[p->get_uuid()] = p;
	}

	void state::remove_player(player_ptr p)
	{
		auto it = players_.find(p->get_uuid());
		ASSERT_LOG(it != players_.end(), "Attempted to remove player " << p->name() << " failed, player doesn't exist.");
		players_.erase(it);
	}

	void state::replace_player(player_ptr to_be_replaced, player_ptr replacement)
	{
		auto it = players_.find(to_be_replaced->get_uuid());
		ASSERT_LOG(it != players_.end(), "Attempted to remove player " << to_be_replaced->name() << " failed, player doesn't exist.");

		// need to change the player in all entities.
		for(auto& e : entities_) {
			auto owner = e->owner.lock();
			if(owner == it->second) {
				e->owner = replacement;
			}
		}

		// remove player from list and add replacement.
		players_.erase(it);
		players_[replacement->get_uuid()] = replacement;
	}

	player_ptr state::get_player(const uuid::uuid& n)
	{
		auto it = players_.find(n);
		ASSERT_LOG(it != players_.end(), "Requested player " << uuid::write(n) << " not found.");
		return players_[n];
	}

	player_weak_ptr state::get_current_player() const
	{
		// XXX strictly this isn't an error and i need a better way of dealing with it.
		ASSERT_LOG(entities_.size() > 0, "No current units.");
		return entities_.front()->owner;
	}

	player_ptr state::get_player_by_id(int id)
	{
		/// XXX don't really like this.
		for(auto& p : players_) {
			if(p.second->id() == id) {
				return p.second;
			}
		}
		return nullptr;
	}

	Update* state::unit_summon(component_set_ptr e)
	{
		// Generate a message to be sent to the server
		return nullptr;
	}

	Update* state::unit_move(component_set_ptr e, const std::vector<point>& path)
	{
		// Generate a message to be sent to the server
		Update* u = new Update();
		u->set_id(update_counter_);
		Update_Unit *unit = u->add_units();
		unit->set_uuid(uuid::write(e->entity_id));
		unit->set_type(Update_Unit_MessageType::Update_Unit_MessageType_MOVE);
		for(auto& p : path) {
			Update_Location* loc = unit->add_path();
			loc->set_x(p.x);
			loc->set_y(p.y);
		}
		// XXX Set any animation hints as required here.
		return u;
	}

	std::vector<Update*> state::validate_and_apply(Update* up)
	{
		std::vector<Update*> res;

		if(up->id() <= update_counter_) {
			// XXX we should resend the complete state as this update seems old.
			//res.emplace_back(generate_complete());
			return res;
		}

		for(auto& players : up->player()) {
			// XXX deal with stuff
			switch(players.action())
			{
				case Update_Player_Action_NONE:
				case Update_Player_Action_JOIN:
				case Update_Player_Action_QUIT:
				case Update_Player_Action_CONCEDE:
					break;
				default: 
					ASSERT_LOG(false, "Unrecognised player.action() value: " << players.action());
			}
		}

		for(auto& units : up->units()) {
			switch(units.type())
			{
				case Update_Unit_MessageType_PASS:
				case Update_Unit_MessageType_SUMMON:
					break;
				case Update_Unit_MessageType_MOVE:
					// Validate that the unit has enough move to afford going along the given path.
					// XXX
					//if(validate_move(units.uuid(), units.path()) {
					//	res.
					//}
					break;
				default: 
					ASSERT_LOG(false, "Unrecognised units.type() value: " << units.type());
			}
		}

		return res;
	}
}
