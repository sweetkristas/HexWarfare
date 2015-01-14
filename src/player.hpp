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

#include <memory>
#include <string>

#include "asserts.hpp"
#include "color.hpp"
#include "uuid.hpp"

enum class PlayerType {
	NORMAL,
	AI,
};

class player;
typedef std::shared_ptr<player> player_ptr;
typedef std::weak_ptr<player> player_weak_ptr;

namespace game
{
	class state;
	class Update;
}

class team
{
public:
	team(const std::string& name, const uuid::uuid& id=uuid::generate()) : team_name_(name), team_(id) {
	}
	const uuid::uuid& id() const { return team_; }
	const std::string& get_team_name() const { return team_name_; }
	void set_team_name(const std::string& name) { team_name_ = name; }
private:
	// Name of the team that the player is on.
	std::string team_name_;
	// players with the same team value are on the same team.
	uuid::uuid team_;
};

typedef std::shared_ptr<team> team_ptr;

inline bool operator==(const team& lhs, const team& rhs) {
	return lhs.id() == rhs.id();
}
inline bool operator!=(const team& lhs, const team& rhs) {
	return !operator==(lhs, rhs);
}
inline bool operator==(const team_ptr& lhs, const team_ptr& rhs) {
	return lhs->id() == rhs->id();
}
inline bool operator!=(const team_ptr& lhs, const team_ptr& rhs) {
	return !operator==(lhs, rhs);
}

class player
{
public:
	explicit player(team_ptr team, PlayerType pt, const std::string& name, uuid::uuid u=uuid::generate());
	virtual ~player();
	const std::string& name() const { return name_; }
	PlayerType player_type() const { return player_type_; }
	bool is_human() const { return player_type_ == PlayerType::NORMAL; }
	bool is_ai() const { return player_type_ == PlayerType::AI; }
	const team_ptr& team() const { ASSERT_LOG(team_ != nullptr, "team was null"); return team_; }
	const graphics::color get_color() const { return color_; }
	void set_color(graphics::color color) { color_ = color; }
	const uuid::uuid& get_uuid() const { return uuid_; }
	int id() const { return id_; }
	void set_id(int id) { id_ = id; }

	virtual game::Update* process(const game::state& gs, double time) { return nullptr; }

	virtual player_ptr clone();
private:
	PlayerType player_type_;
	std::string name_;
	team_ptr team_;
	// Color 
	graphics::color color_;
	uuid::uuid uuid_;

	// simple id, still debating this.
	int id_;
};
