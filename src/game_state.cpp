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
#include "formatter.hpp"
#include "game_state.hpp"
#include "profile_timer.hpp"

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
		u->set_id(++update_counter_);
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

		if(up->id() < update_counter_) {
			// XXX we should resend the complete state as this update seems old.
			//res.emplace_back(generate_complete());
			return res;
		}

		for(auto& players : up->player()) {
			// XXX deal with stuff
			switch(players.action())
			{
				case Update_Player_Action_CANONICAL_STATE:
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
				case Update_Unit_MessageType_CANONICAL_STATE:
					// we should never ever recieve this message from a client!
					ASSERT_LOG(false, "Got unit 'STATE' message from client. This is an error in the client code.");
					break;
				case Update_Unit_MessageType_SUMMON:
					break;
				case Update_Unit_MessageType_MOVE: {
					// Validate that the unit has enough move to afford going along the given path.
					auto e = get_entity_by_uuid(uuid::read(units.uuid()));
					if(validate_move(e, units.path())) {
					//	res.
						++update_counter_;
						// XXX send the path update to all the clients here.
						up->set_id(update_counter_);
						res.emplace_back(up);
					} else {
						// The path provided has a cost which is more than the number of move left.
						// XXX Re-send the complete game state.
					}
					break;
				}
				case Update_Unit_MessageType_PASS:
					break;
				default: 
					ASSERT_LOG(false, "Unrecognised units.type() value: " << units.type());
			}
		}

		return res;
	}

	component_set_ptr state::get_entity_by_uuid(const uuid::uuid& id)
	{
		auto it = std::find_if(entities_.begin(), entities_.end(), [&id](component_set_ptr e){
			return e->entity_id == id;
		});
		ASSERT_LOG(it != entities_.end(), "Couldn't find entity with uuid: " << id);
		return *it;
	}

	void state::set_validation_fail_reason(const std::string& reason)
	{
		fail_reason_ = reason;
	}

	bool state::validate_move(component_set_ptr e, const ::google::protobuf::RepeatedPtrField<Update_Location>& path)
	{
		profile::manager pman("state::validate_move");
		// check that it is the turn of e to move/action.
		if(entities_.front() != e) {
			set_validation_fail_reason(formatter() << "entity(" << e->entity_id << ") wasn't the current unit with initiative(" << entities_.front()->entity_id << ").");
			return false;
		}

		std::set<point> enemy_locations;
		std::set<point> zoc_locations;
		// Create sets of enemy locations and tiles under zoc
		auto e1_owner = e->owner.lock();
		for(auto entity : entities_) {
			auto pos = entity->pos->pos;
			auto e2_owner = entity->owner.lock();
			if(e1_owner->team() != e2_owner->team()) {
				enemy_locations.emplace(pos);
				for(auto& p : map_->get_surrounding_positions(pos)) {
					zoc_locations.emplace(p);
				}
			}
		}
		// remove enemy locations from zoc
		for(auto& p : enemy_locations) {
			auto it = zoc_locations.find(p);
			if(it != zoc_locations.end()) {
				zoc_locations.erase(it);
			}
		}
		float cost(0);
		for(auto& p : path) {
			point pp(p.x(), p.y());
			auto tile = map_->get_tile_at(p.x(), p.y());
			cost += tile->get_cost();

			auto it = enemy_locations.find(pp);
			if(it != enemy_locations.end()) {
				set_validation_fail_reason(formatter() << "Enemy unit exists in given path at (" << p.x() << "," << p.y() << ")");
				return false;
			}
			// check that if we pass into a ZoC tile then we stop, i.e. no ZoC tiles mid-path.
			auto zit = zoc_locations.find(pp);
			if(zit != zoc_locations.end() && (pp.x != path.end()->x() && pp.y != path.end()->y())) {
				set_validation_fail_reason(formatter() << "ZOC tile at (" << p.x() << "," << p.y() << ") was in middle of path.");
				return false;
			}
		}
		if(e->stat->move >= cost) {
			e->stat->move -= cost;
			if(e->stat->move < FLT_EPSILON) {
				e->stat->move = 0;
			}
			e->pos->pos.x = path.end()->x();
			e->pos->pos.y = path.end()->y();
			return true;
		}
		set_validation_fail_reason(formatter() << "Unit didn't have enough movement left. " << e->stat->move << cost);
		return false;
	}
}
