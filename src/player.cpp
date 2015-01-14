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

#include "player.hpp"

player::player(team_ptr team, PlayerType pt, const std::string& name, uuid::uuid u)
	: player_type_(pt),
	  name_(name),
	  team_(team),
	  uuid_(u)

{
}

player::~player()
{
}

player_ptr player::clone()
{
	return player_ptr(new player(*this));
}
