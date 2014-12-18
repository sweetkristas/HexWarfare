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
#include "game_state.hpp"
#include "hex_logical_fwd.hpp"

namespace hex
{
	typedef float cost;
	typedef point node_type;
	typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, boost::property<boost::edge_weight_t, cost>> hex_graph;
	typedef boost::property_map<hex_graph, boost::edge_weight_t>::type WeightMap;
	typedef hex_graph::vertex_descriptor vertex;
	typedef hex_graph::edge_descriptor edge_descriptor;
	typedef std::pair<point, point> edge;

	struct graph_t
	{
		graph_t(size_t size) : graph(size) {}
		hex_graph graph;
		std::map<point, int> reverse_map;
		std::vector<point> vertices;
	};
	typedef std::shared_ptr<graph_t> hex_graph_ptr;

	typedef std::vector<point> result_path;

	hex_graph_ptr create_cost_graph(const game::state& gs, int srcx, int srcy, float max_cost);
	hex_graph_ptr create_graph(const game::state& gs, int x=0, int y=0, int w=0, int h=0);
	result_list find_available_moves(hex_graph_ptr graph, const point& src, float max_cost);
	result_path find_path(hex_graph_ptr graph, const point& src, const point& dst);
}
