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

#include <tuple>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "component.hpp"
#include "engine.hpp"
#include "hex_map.hpp"

namespace hex
{
	typedef float cost;
	typedef const hex_object* node_type;
	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost>> hex_graph;
	typedef boost::property_map<hex_graph, boost::edge_weight_t>::type WeightMap;
	typedef hex_graph::vertex_descriptor vertex;
	typedef hex_graph::edge_descriptor edge_descriptor;
	typedef std::pair<const hex_object*, const hex_object*> edge;

	struct graph_t
	{
		graph_t(size_t size) : graph(size) {}
		hex_graph graph;
		std::map<const hex_object*, int> reverse_map;
		std::vector<const hex_object*> vertices;
	};
	typedef std::shared_ptr<graph_t> hex_graph_ptr;

	hex_graph_ptr create_cost_graph(const engine& eng, hex_map_ptr map, int srcx, int srcy, float max_cost);
	hex_graph_ptr create_graph(const engine& eng, hex_map_ptr map, int x=0, int y=0, int w=0, int h=0);
	result_list find_available_moves(hex_graph_ptr graph, const hex_object* src, float max_cost);
	result_path find_path(hex_graph_ptr graph, const hex_object* src, const hex_object* dst);
}
