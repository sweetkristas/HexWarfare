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

	state::state(const state& obj)
		: initiative_counter_(obj.initiative_counter_),
		  update_counter_(obj.update_counter_),
		  map_(obj.map_->clone())
	{
		for(auto& p : obj.players_) {
			players_[p.first] = p.second->clone();
		}
		for(auto& e : obj.entities_) {
			auto owner = e->owner.lock();
			ASSERT_LOG(owner != nullptr, "Couldn't lock owner of entity " << e);
			auto it = players_.find(owner->get_uuid());
			ASSERT_LOG(it != players_.end(), "Couldn't find owner for entity: " << e);
			entities_.emplace_back(e->clone(it->second));
		}
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
			// reset the movement for the unit at the front of the list.
			e->stat->move = e->stat->unit->get_movement();
			// update the unit at the front of the list initiative.
			e->stat->initiative += 100.0f/e->stat->unit->get_initiative();
			std::stable_sort(entities_.begin(), entities_.end(), component::initiative_compare);
			initiative_counter_ = entities_.front()->stat->initiative;
		}

		//std::cerr << "Initative list:";
		//for(auto& e : entities_) {
		//	std::cerr << "\t" << e->stat->name << ":" << e->stat->initiative << " : move(" << e->stat->move << ")\n";
		//}
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

	Update* state::create_update() const
	{
		Update* up = new Update();
		up->set_id(++update_counter_);
		return up;
	}

	const state& state::end_turn(Update* up) const 
	{
		up->set_end_turn(true);
		return *this;
	}

	const state& state::unit_summon(Update* up, component_set_ptr e) const
	{
		// Generate a message to be sent to the server
		return *this;
	}

	const state& state::unit_move(Update* up, component_set_ptr e, const std::vector<point>& path) const 
	{
		// Generate a message to be sent to the server
		Update_Unit *unit = up->add_units();
		unit->set_uuid(uuid::write(e->entity_id));
		unit->set_type(Update_Unit_MessageType::Update_Unit_MessageType_MOVE);
		for(auto& p : path) {
			Update_Location* loc = unit->add_path();
			loc->set_x(p.x);
			loc->set_y(p.y);
		}
		return *this;
	}

	const state& state::unit_attack(Update* up, const component_set_ptr& e, const std::vector<component_set_ptr>& targets) const 
	{
		Update_Unit *unit = up->add_units();
		unit->set_uuid(uuid::write(e->entity_id));
		unit->set_type(Update_Unit_MessageType::Update_Unit_MessageType_ATTACK);
		for(auto& t : targets) {
			std::string* str = unit->add_target_uuids();
			*str = uuid::write(t->entity_id);
		}
		return *this;
	}

	Update* state::validate_and_apply(Update* up)
	{
		if(up->has_quit() && up->quit() && up->id() == -1) {
			Update* nup = new Update();
			nup->set_id(-1);
			nup->set_quit(true);
			return nup;
		}

		if(up->id() < update_counter_) {
			// XXX we should resend the complete state as this update seems old.
			//res.emplace_back(generate_complete());
			LOG_WARN("Got old update: " << up->id() << " : " << update_counter_);
			return nullptr;
		}

		// Create a new update to be sent
		auto nup = create_update();

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
			// Create a new unit based on this current one.
			Update_Unit* uu = nup->add_units();
			uu->set_uuid(units.uuid());
			uu->set_type(Update_Unit_MessageType_PASS);

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
						// send path to clients
						uu->set_type(Update_Unit_MessageType::Update_Unit_MessageType_MOVE);
						// XXX see if there is a better way of moving the path using protobufs
						for(auto& p : units.path()) {
							Update_Location* loc = uu->add_path();
							loc->set_x(p.x());
							loc->set_y(p.y());
						}
					} else {
						// The path provided has a cost which is more than the number of move left.
						// XXX Re-send the complete game state.
						// nup->set_fail_reason(fail_reason_);
						LOG_WARN("Failed to validate move: " << fail_reason_);
					}
					break;
				}
				case Update_Unit_MessageType_ATTACK: {
					auto aggressor = get_entity_by_uuid(uuid::read(units.uuid()));
					for(auto& target_id : units.target_uuids()) {
						auto t = get_entity_by_uuid(uuid::read(target_id));
						if(is_attackable(aggressor, t)) {
							combat(nup, aggressor, t);
						} else {
							LOG_WARN(t << " couldn't be attacked.");
						}
					}
					break;
				}
				case Update_Unit_MessageType_SPELL: {
					break;
				}
				case Update_Unit_MessageType_PASS:
					break;
				default: 
					ASSERT_LOG(false, "Unrecognised units.type() value: " << units.type());
			}
		}

		if(up->has_end_turn() && up->end_turn()) {
			end_unit_turn();
			nup->set_end_turn(true);
		}

		// Check for victory condition -- assumes it is one side losing all their units.
		if(entities_.size() == 0) {
			// all units killed during this turn -- calling it a draw.
			nup->set_game_win_state(Update_GameWinState_DRAW);
		} else {
			// check to see if all entities on one side are dead.
			// XXX this feels like a horrble over-kill hacky way of doing it.
			std::map<uuid::uuid,int> score;
			for(auto& e : entities_) { 
				const uuid::uuid& id = e->owner.lock()->team()->id();
				auto it = score.find(id);
				if(it == score.end()) {
					score[id] = 1;
				} else {
					(it->second)++;
				}
			}
			team_ptr winning_team = entities_.front()->owner.lock()->team();
			if(score.size() == 1) {
				nup->set_winning_team_uuid(uuid::write(winning_team->id()));
				nup->set_game_win_state(Update_GameWinState_WON);
			}
		}
		return nup;
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
			set_validation_fail_reason(formatter() << e << " wasn't the current unit with initiative " << entities_.front() << " was.");
			return false;
		}

		LOG_DEBUG("Validate move: " << e);

		std::set<point> enemy_locations;
		std::set<point> zoc_locations;
		// Create sets of enemy locations and tiles under zoc
		auto e1_owner = e->owner.lock();
		for(auto entity : entities_) {
			auto pos = entity->pos->gs_pos;
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
		auto p = path.begin();
		bool last_tile_zoc = zoc_locations.find(point(p->x(), p->y())) != zoc_locations.end();
		++p;
		for(; p != path.end(); ++p) {
			point pp(p->x(), p->y());
			auto tile = map_->get_tile_at(p->x(), p->y());
			ASSERT_LOG(tile != nullptr, "No tile exists at point: " << pp);
			cost += tile->get_cost();
			//LOG_DEBUG("tile" << pp << ": " << tile->name() << " : " << tile->get_cost());

			auto it = enemy_locations.find(pp);
			if(it != enemy_locations.end()) {
				set_validation_fail_reason(formatter() << "Enemy unit exists in given path at " << pp);
				return false;
			}
			// check that if we pass into a ZoC tile then we stop, i.e. no ZoC tiles mid-path.
			auto zit = zoc_locations.find(pp);
			if(zit != zoc_locations.end() && (pp.x != path.rbegin()->x() && pp.y != path.rbegin()->y())) {
				set_validation_fail_reason(formatter() << "ZOC tile at " << pp << " was in middle of path.");
				return false;
			}
		}
		if(e->stat->move >= cost) {
			e->stat->move -= cost;
			if(e->stat->move < FLT_EPSILON) {
				e->stat->move = 0;
			}
			e->pos->gs_pos.x = path.rbegin()->x();
			e->pos->gs_pos.y = path.rbegin()->y();
			return true;
		}
		set_validation_fail_reason(formatter() << "Unit didn't have enough movement left. " << e->stat->move << cost);
		return false;
	}

	bool state::is_attackable(const component_set_ptr& aggressor, const component_set_ptr& e) const
	{
		if(aggressor == e) {
			LOG_INFO(aggressor << " could not attack target, same unit");
			return false;
		}
		if(aggressor->owner.lock()->team() == e->owner.lock()->team()) {
			// Don't let us attack units on the same team
			// XXX it may be a legitimate tactic to target units on your own team
			// if this is the case then they should be distinguished from (say yellow) from
			// enemy units (red).
			return false;
		}
		int d = hex::logical::distance(aggressor->pos->gs_pos, e->pos->gs_pos);
		if(d > aggressor->stat->range) {
			LOG_INFO(aggressor << " could not attack target " << e << " distance too great: " << d);
			return false;
		}

		if(d > 1 /*&& !aggressor->stat->unit->has_ability("strike-through")*/) {
			// Find the direct line between the two units
			// make sure that there are no other entities in the way, unless the unit has the
			// "strike-through" ability.
			auto line = hex::logical::line(aggressor->pos->gs_pos, e->pos->gs_pos);
			// remove first and last elements from the line.
			line.pop_back();
			line.erase(line.begin());
			for(auto& p : line) {
				for(auto& en : entities_) {
					if(p == en->pos->gs_pos) {
						LOG_INFO(aggressor << " could not attack target " << e << " unit in path " << en);
						return false;
					}
				}
			}
		}

		// XXX add other checks here based on terrain and if say the defender is invulnerable to attack.
		return true;
	}

	void state::apply(Update* up)
	{
		// client side update
		update_counter_ = up->id();
		if(up->has_fail_reason()) {
			LOG_WARN("Server failed last command. Reason: " << up->fail_reason());
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
					break;
				case Update_Unit_MessageType_SUMMON:
					break;
				case Update_Unit_MessageType_MOVE: {
					auto e = get_entity_by_uuid(uuid::read(units.uuid()));
					auto p = units.path().end() - 1;
					auto start_p = point(units.path().begin()->x(), units.path().begin()->y());
					LOG_INFO("moving " << e << " from " << start_p << " to position " << point(p->x(), p->y()));
					e->pos->gs_pos.x = p->x();
					e->pos->gs_pos.y = p->y();
					break;
				}
				case Update_Unit_MessageType_ATTACK: {
					// Attack message type means we need to look at the stat values and update
					// If the unit has no health left it's considered dead.
					auto e = get_entity_by_uuid(uuid::read(units.uuid()));
					ASSERT_LOG(units.has_stats(), "No stats block attached to attack");
					set_entity_stats(e, units.stats());
					// clear attack targets
					for(auto& unit : entities_) {
						if(unit->inp) {
							unit->inp->is_attack_target = false;
						}
					}
					break;
				}
				case Update_Unit_MessageType_SPELL: {
					break;
				}
				case Update_Unit_MessageType_PASS:
					break;
				default: 
					ASSERT_LOG(false, "Unrecognised units.type() value: " << units.type());
			}
		}

		if(up->has_end_turn() && up->end_turn()) {
			// do client side end turn.
			end_unit_turn();
		}
	}

	void state::combat(Update* up, component_set_ptr aggressor, component_set_ptr target)
	{
		ASSERT_LOG(up != nullptr, "game logic bug Update is null.");
		auto& a_stat = aggressor->stat;
		auto& t_stat = target->stat;
		if(a_stat->attack > t_stat->armour) {
			t_stat->health -= a_stat->attack - t_stat->armour;
			LOG_INFO(target << " takes " << (a_stat->attack - t_stat->armour) << " damage.");
			if(t_stat->health < 0) {
				LOG_INFO(target << " dies due to a fatal wound.");
			}
		} else {
			LOG_INFO(target << " takes no damage due to high armour.");
		}

		Update_Unit* unit = up->add_units();
		unit->set_uuid(uuid::write(target->entity_id));
		Update_UnitStats* uus = new Update_UnitStats();
		uus->set_health(t_stat->health);
		unit->set_allocated_stats(uus);
		unit->set_type(Update_Unit_MessageType_ATTACK);

		// XXX If we were doing a retalitory strike we could add code here.
		// Might pay to pass in the aggressor Update_Unit* pointer.


		// Remove either unit if health is below zero.
		if(target->stat->health <= 0) {
			remove_entity(target);
		}
	}

	void state::set_entity_stats(component_set_ptr e, const Update_UnitStats& stats)
	{
		auto& stat = e->stat;
		if(stats.has_armour()) {
			stat->armour = stats.armour();
		}
		if(stats.has_attack()) {
			stat->attack = stats.attack();
		}
		if(stats.has_health()) {
			stat->health = stats.health();
		}
		if(stats.has_initiative()) {
			stat->initiative = stats.initiative();
		}
		if(stats.has_move()) {
			stat->move = stats.move();
		}
		if(stats.has_name()) {
			stat->name = stats.name();
		}
		if(stats.has_range()) {
			stat->range = stats.range();
		}
	}

	team_ptr state::create_team_instance(const std::string& name)
	{
		auto t = std::make_shared<team>(name);
		teams_[t->id()] = t;
		return t;
	}

	team_ptr state::get_team_from_id(const uuid::uuid& id)
	{
		auto it = teams_.find(id);
		ASSERT_LOG(it != teams_.end(), "Couldn't find team for id: " << id);
		return it->second;
	}
}
