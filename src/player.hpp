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

enum class PlayerType {
	NORMAL,
	AI,
};

class player
{
public:
	explicit player(PlayerType pt, const std::string& name);
	const std::string& name() const { return name_; }
	PlayerType player_type() const { return player_type_; }
	bool is_human() const { return player_type_ == PlayerType::NORMAL; }
	bool is_ai() const { return player_type_ == PlayerType::AI; }
private:
	PlayerType player_type_;
	std::string name_;
};

typedef std::shared_ptr<player> player_ptr;
typedef std::weak_ptr<player> player_weak_ptr;