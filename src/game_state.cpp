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
#include "creature.hpp"
#include "formatter.hpp"
#include "game_state.hpp"
#include "hex_logical_tiles.hpp"
#include "profile_timer.hpp"
#include "random.hpp"
#include "units.hpp"
#include "uuid.hpp"

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
		for(auto& u : obj.units_) {
			auto owner = u->get_owner();
			ASSERT_LOG(owner != nullptr, "Couldn't lock owner of " << u);
			auto it = players_.find(owner->get_uuid());
			ASSERT_LOG(it != players_.end(), "Couldn't find owner for " << u);
			units_.emplace_back(u->clone(it->second));
		}
	}

	state::~state()
	{
	}

	void state::set_map(hex::logical::map_ptr map)
	{
		map_ = map;
	}

	unit_ptr state::create_unit_instance(const std::string& type, const player_ptr& pid, const point& pos)
	{
		return creature::spawn(*this, type, pid, pos);
	}

	void state::add_unit(unit_ptr e)
	{
		units_.emplace_back(e);
		std::stable_sort(units_.begin(), units_.end(), initiative_compare);
	}

	void state::remove_unit(unit_ptr e1)
	{
		units_.erase(std::remove_if(units_.begin(), units_.end(), [&e1](unit_ptr e2) {
			return e1 == e2; 
		}), units_.end());
	}

	void state::end_unit_turn()
	{
		if(units_.size() > 0) {
			units_.front()->complete_turn();
			std::stable_sort(units_.begin(), units_.end(), initiative_compare);
			initiative_counter_ = units_.front()->get_initiative();
			units_.front()->start_turn();
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
		for(auto& u : units_) {
			auto owner = u->get_owner();
			if(owner == it->second) {
				u->set_owner(replacement);
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

	player_ptr state::get_current_player() const
	{
		// XXX strictly this isn't an error and i need a better way of dealing with it.
		ASSERT_LOG(units_.size() > 0, "No current units.");
		return units_.front()->get_owner();
	}

	std::vector<player_ptr> state::get_players()
	{
		std::vector<player_ptr> res;
		for(auto& p : players_) {
			res.emplace_back(p.second);
		}
		return res;
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

	const state& state::unit_summon(Update* up, unit_ptr e) const
	{
		// Generate a message to be sent to the server
		return *this;
	}

	const state& state::unit_move(Update* up, unit_ptr u, const std::vector<point>& path) const 
	{
		// Generate a message to be sent to the server
		Update_Unit *unit = up->add_units();
		unit->set_uuid(uuid::write(u->get_uuid()));
		unit->set_type(Update_Unit_MessageType::Update_Unit_MessageType_MOVE);
		float cost(0);
		for(auto& p : path) {
			Update_Location* loc = unit->add_path();
			loc->set_x(p.x);
			loc->set_y(p.y);
			cost -= map_->get_tile_at(p)->get_cost();
		}
		// Set the game state position.
		u->set_position(path.back());
		u->set_move(u->get_move() - cost);
		return *this;
	}

	const state& state::unit_attack(Update* up, const unit_ptr& e, const std::vector<unit_ptr>& targets) const 
	{
		Update_Unit *unit = up->add_units();
		unit->set_uuid(uuid::write(e->get_uuid()));
		unit->set_type(Update_Unit_MessageType::Update_Unit_MessageType_ATTACK);
		for(auto& t : targets) {
			std::string* str = unit->add_target_uuids();
			*str = uuid::write(t->get_uuid());
		}
		// N.B. adjusting the game state stuff in the engine is slightly hackish. But when the
		// server responds with an actual update this should be corrected.
		e->dec_attacks_this_turn();
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
				case Update_Player_Action_ELIMINATED:
					LOG_ERROR("The server should never receive a player eliminated message from the clients");
					break;
				case Update_Player_Action_UPDATE:
					LOG_ERROR("The server should never receive a player update message from the clients");
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
					auto e = get_unit_by_uuid(uuid::read(units.uuid()));
					if(validate_move(e, units.path())) {
						// send path to clients
						uu->set_type(Update_Unit_MessageType::Update_Unit_MessageType_MOVE);
						// XXX see if there is a better way of moving the path using protobufs
						for(auto& p : units.path()) {
							Update_Location* loc = uu->add_path();
							loc->set_x(p.x());
							loc->set_y(p.y());
						}
						// Make sure we set the units actual movement.
						Update_UnitStats* uus = new Update_UnitStats();
						uus->set_move(e->get_move());
						uu->set_allocated_stats(uus);
					} else {
						// The path provided has a cost which is more than the number of move left.
						// XXX Re-send the complete game state.
						// nup->set_fail_reason(fail_reason_);
						LOG_WARN("Failed to validate move: " << fail_reason_);
					}
					break;
				}
				case Update_Unit_MessageType_ATTACK: {
					auto aggressor = get_unit_by_uuid(uuid::read(units.uuid()));
					for(auto& target_id : units.target_uuids()) {
						auto t = get_unit_by_uuid(uuid::read(target_id));
						if(is_attackable(aggressor, t)) {
							combat(nup, uu, aggressor, t);
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
		if(units_.size() == 0) {
			// all units killed during this turn -- calling it a draw.
			nup->set_game_win_state(Update_GameWinState_DRAW);
		} else {
			// check to see if all entities on one side are dead.
			// XXX this feels like a horrble over-kill hacky way of doing it.
			std::map<uuid::uuid,int> score;
			for(auto& e : units_) { 
				const uuid::uuid& id = e->get_owner()->team()->id();
				auto it = score.find(id);
				if(it == score.end()) {
					score[id] = 1;
				} else {
					(it->second)++;
				}
			}
			team_ptr winning_team = units_.front()->get_owner()->team();
			if(score.size() == 1) {
				nup->set_winning_team_uuid(uuid::write(winning_team->id()));
				nup->set_game_win_state(Update_GameWinState_WON);
			}
		}
		return nup;
	}

	unit_ptr state::get_unit_by_uuid(const uuid::uuid& id)
	{
		auto it = std::find_if(units_.begin(), units_.end(), [&id](unit_ptr u){
			return u->get_uuid() == id;
		});
		ASSERT_LOG(it != units_.end(), "Couldn't find unit with uuid: " << uuid::write(id));
		return *it;
	}

	void state::set_validation_fail_reason(const std::string& reason)
	{
		fail_reason_ = reason;
	}

	bool state::validate_move(const unit_ptr& u, const ::google::protobuf::RepeatedPtrField<Update_Location>& path)
	{
		profile::manager pman("state::validate_move");
		// check that it is the turn of e to move/action.
		if(units_.front() != u) {
			set_validation_fail_reason(formatter() << u << " wasn't the current unit with initiative " << units_.front() << " was.");
			return false;
		}

		LOG_DEBUG("Validate move: " << u);

		std::set<point> enemy_locations;
		std::set<point> zoc_locations;
		// Create sets of enemy locations and tiles under zoc
		auto e1_owner = u->get_owner();
		for(auto entity : units_) {
			auto pos = entity->get_position();
			auto e2_owner = entity->get_owner();
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
		if(u->get_move() >= cost) {
			u->set_move(u->get_move() - cost);
			if(u->get_move() < FLT_EPSILON) {
				u->set_move(0);
			}
			u->set_position(path.rbegin()->x(), path.rbegin()->y());
			return true;
		}
		set_validation_fail_reason(formatter() << "Unit didn't have enough movement left. " << u->get_move() << " : " << cost);
		return false;
	}

	bool state::is_attackable(const unit_ptr& aggressor, const unit_ptr& e) const
	{
		if(aggressor == e) {
			LOG_INFO(aggressor << " could not attack target, same unit");
			return false;
		}
		if(aggressor->get_owner()->team() == e->get_owner()->team()) {
			// Don't let us attack units on the same team
			// XXX it may be a legitimate tactic to target units on your own team
			// if this is the case then they should be distinguished from (say yellow) from
			// enemy units (red).
			return false;
		}
		int d = hex::logical::distance(aggressor->get_position(), e->get_position());
		if(d > aggressor->get_range()) {
			LOG_INFO(aggressor << " could not attack target " << e << " distance too great: " << d);
			return false;
		}

		if(d > 1 /*&& !aggressor->stat->unit->has_ability("strike-through")*/) {
			// Find the direct line between the two units
			// make sure that there are no other entities in the way, unless the unit has the
			// "strike-through" ability.
			auto line = hex::logical::line(aggressor->get_position(), e->get_position());
			// remove first and last elements from the line.
			line.pop_back();
			line.erase(line.begin());
			for(auto& p : line) {
				for(auto& en : units_) {
					// XXX The commented out code allows you to attack through your own team members.
					// It may be annoying to not allow this, in practice.
					if(p == en->get_position() /*&& en->get_owner()->team() != aggressor->get_owner()->team()*/) {
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
			auto p = get_player_by_uuid(uuid::read(players.uuid()));
			switch(players.action())
			{
				case Update_Player_Action_CANONICAL_STATE:
				case Update_Player_Action_JOIN:
				case Update_Player_Action_QUIT:
				case Update_Player_Action_CONCEDE:
				case Update_Player_Action_ELIMINATED:
					break;
				case Update_Player_Action_UPDATE: {
					ASSERT_LOG(players.has_player_info(), "Client received player update message with no attached player_info");
					const Update_PlayerInfo& pi = players.player_info();
					if(pi.has_gold()) {
						p->set_gold(pi.gold());
					}
					break;
				}
				default: 
					ASSERT_LOG(false, "Unrecognised player.action() value: " << players.action());
			}
		}

		for(auto& units : up->units()) {
			auto e = get_unit_by_uuid(uuid::read(units.uuid()));
			if(units.has_stats()) {
				set_unit_stats(e, units.stats());
			}

			switch(units.type())
			{
				case Update_Unit_MessageType_CANONICAL_STATE:
					break;
				case Update_Unit_MessageType_SUMMON:
					break;
				case Update_Unit_MessageType_MOVE: {
					auto p = units.path().end() - 1;
					auto start_p = point(units.path().begin()->x(), units.path().begin()->y());
					LOG_INFO("moving " << e << " from " << start_p << " to position " << point(p->x(), p->y()));
					e->set_position(p->x(), p->y());
					break;
				}
				case Update_Unit_MessageType_ATTACK: {
					// Attack message type means we need to look at the stat values and update
					// If the unit has no health left it's considered dead.
					ASSERT_LOG(units.has_stats(), "No stats block attached to attack");
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

			if(e->get_health() <= 0) {
				remove_unit(e);
			}
		}

		if(up->has_end_turn() && up->end_turn()) {
			// do client side end turn.
			end_unit_turn();
		}
	}

	void state::combat(Update* up, Update_Unit* agg_uu, unit_ptr aggressor, unit_ptr target)
	{
		ASSERT_LOG(up != nullptr, "game logic bug Update is null.");
		if(aggressor->get_attacks_this_turn() <= 0) {
			LOG_WARN(aggressor << " has no attacks left this turn " << aggressor->get_attacks_this_turn());
			return;
		}
		Update_AttackInfo* uai = nullptr;
		if(aggressor->get_attack() > target->get_armour()) {
			const bool was_critical = generator::get_uniform_real<float>(0.0f,1.0f) < aggressor->get_critical_strike();
			// XXX We need to note that a critical strike occurred with an animation of some sort.
			const int damage = (aggressor->get_attack() - target->get_armour()) * (was_critical ? 2 : 1);
			target->set_health(target->get_health() - damage);
			LOG_INFO(target << " takes " << damage << (was_critical ? " critical" : "") << " damage. ");
			if(target->get_health() < 0) {
				LOG_INFO(target << " dies due to a fatal wound.");
			}
			uai = new Update_AttackInfo();
			uai->set_was_critical(was_critical);
			uai->set_damage(damage);
		} else {
			LOG_INFO(target << " takes no damage due to high armour.");
		}

		Update_Unit* unit = up->add_units();
		unit->set_uuid(uuid::write(target->get_uuid()));
		Update_UnitStats* uus = new Update_UnitStats();
		uus->set_health(target->get_health());
		unit->set_allocated_stats(uus);
		if(uai) {
			unit->set_allocated_attack_info(uai);
		}
		unit->set_type(Update_Unit_MessageType_ATTACK);

		// XXX If we were doing a retalitory strike we could add code here.
		// Might pay to pass in the aggressor Update_Unit* pointer.

		aggressor->dec_attacks_this_turn();
		Update_UnitStats* agg_uus = nullptr;
		if(agg_uu->has_stats()) {
			agg_uus = agg_uu->mutable_stats();
		} else {
			agg_uus = new Update_UnitStats();
			agg_uu->set_allocated_stats(agg_uus);
		}
		agg_uus->set_attacks_this_turn(aggressor->get_attacks_this_turn());

		// Remove either unit if health is below zero.
		if(target->get_health() <= 0) {
			remove_unit(target);
		}
	}

	void state::set_unit_stats(unit_ptr u, const Update_UnitStats& stats)
	{
		if(stats.has_armour()) {
			u->set_armour(stats.armour());
		}
		if(stats.has_attack()) {
			u->set_attack(stats.attack());
		}
		if(stats.has_health()) {
			u->set_health(stats.health());
		}
		if(stats.has_initiative()) {
			u->set_initiative(stats.initiative());
		}
		if(stats.has_move()) {
			u->set_move(stats.move());
		}
		if(stats.has_name()) {
			u->set_name(stats.name());
		}
		if(stats.has_range()) {
			u->set_range(stats.range());
		}
		if(stats.has_critical_strike()) {
			u->set_critical_strike(stats.critical_strike());
		}
		if(stats.has_attacks_this_turn()) {
			u->set_attacks_this_turn(stats.attacks_this_turn());
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
		ASSERT_LOG(it != teams_.end(), "Couldn't find team for id: " << uuid::write(id));
		return it->second;
	}

	const player_ptr& state::get_player_by_uuid(const uuid::uuid& id) const
	{
		auto it = players_.find(id);
		ASSERT_LOG(it != players_.end(), "Couldn't find player with id: " << uuid::write(id));
		return it->second;
	}
}
