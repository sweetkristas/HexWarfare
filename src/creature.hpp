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

	void loader(const node& n);

	component_set_ptr spawn(player_weak_ptr owner, const std::string& type, const point& pos);
}
