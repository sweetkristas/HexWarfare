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
#include <vector>

#include "geometry.hpp"
#include "hex_logical_fwd.hpp"

namespace hex 
{
	class hex_map;
	class hex_object;
	class tile_sheet;
	class tile_type;

	typedef std::shared_ptr<hex_object> hex_object_ptr;
	typedef std::shared_ptr<hex_map> hex_map_ptr;
	typedef std::shared_ptr<const tile_sheet> tile_sheet_ptr;
	typedef std::shared_ptr<tile_type> tile_type_ptr;

	struct move_cost
	{
		move_cost(const point& p, float c) : loc(p), path_cost(c) {}
		point loc;
		float path_cost;
	};
	// XXX result_list might be better served as a std::set
	typedef std::vector<move_cost> result_list;

	struct graph_t;
	typedef std::shared_ptr<graph_t> hex_graph_ptr;
}
