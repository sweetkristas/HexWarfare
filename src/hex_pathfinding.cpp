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

#include "hex_pathfinding.hpp"

namespace hex
{
	typedef std::vector<hex_object_ptr> vertex_list;
	typedef pathfinding::DirectedGraph<hex_object_ptr>::GraphEdgeList edge_list;

	typedef pathfinding::WeightedDirectedGraph<hex_object_ptr,float>::EdgeWeights edge_weights;

	hex_graph_ptr create_graph_from_map(hex_map_ptr map)
	{
		vertex_list vertices;
		edge_list edges;
		// Create the vertex list and edge list.
		for(auto tile : map->get_tiles()) {
			vertices.emplace_back(tile);
			edges[tile] = map->get_surrounding_tiles(tile->x(), tile->y());
		}
		pathfinding::DirectedGraph<hex_object_ptr>::Pointer dg = std::make_shared<pathfinding::DirectedGraph<hex_object_ptr>>(&vertices, &edges);

		edge_weights weights;

		/// XXX make weight gragh here
#error make weight graph

		return std::make_shared<pathfinding::WeightedDirectedGraph<hex_object_ptr,float>>(dg, &weights);
	}

	std::vector<hex_object_ptr> cost_search(hex_graph_ptr graph, hex_object_ptr src, float max_cost)
	{
		return pathfinding::path_cost_search<hex_object_ptr,float>(graph, src, max_cost);
	}
}
