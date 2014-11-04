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

#include "component.hpp"
#include "engine.hpp"
#include "hex_map.hpp"
#include "pathfinding.hpp"

namespace hex
{
	typedef pathfinding::WeightedDirectedGraph<const hex_object*, float>::Pointer hex_graph_ptr;

	hex_graph_ptr create_graph_from_map(const engine& eng, const component_set_ptr& x, hex_map_ptr map);
	std::vector<const hex_object*> cost_search(hex_graph_ptr graph, const hex_object* src, float max_cost);
}
