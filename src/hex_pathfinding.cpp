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
#include "profile_timer.hpp"

namespace hex
{
	typedef std::vector<const hex_object*> vertex_list;
	typedef pathfinding::DirectedGraph<const hex_object*>::GraphEdgeList edge_list;

	typedef pathfinding::EdgePair<const hex_object*> edge_pair;
	typedef pathfinding::WeightedDirectedGraph<const hex_object*,float>::EdgeWeights edge_weights;

	// Basic strategy for generating a graph of available spaces to move to.
	// 1. Create the directed graph vertex list, removing any nodes with enemy or friendly creature entities on it.
	// 2. Create a weighted graph, removing any edges which start in the Zone of Control (ZoC), i.e. surrounding hexes
	//    of an enemy team entity.

	// e is the engine we are using to drive this, from this we can get a list of all entities and the current player.
	// x is the entity we are considering. 
	// map is the current map we are considering.
	hex_graph_ptr create_graph_from_map(hex_map_ptr map)
	{
		using namespace component;
		profile::manager pman("create_graph_from_map");
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);

		vertex_list vertices;
		edge_list edges;
		edge_weights weights;

		vertices.reserve(map->get_tiles().size());
		//profile::manager pman("create_graph_from_map - verts");
		// Create the vertex list and edge list.
		for(const auto& tile : map->get_tiles()) {
			const hex_object* t = &tile;
			vertices.emplace_back(t);
			auto surrounds = map->get_surrounding_tiles(tile.x(), tile.y());
			edges[t] = surrounds;
			for(auto& edge : surrounds) {
				// XXX replace 1.0f below with a value for the terrain being traversed to.
				weights[edge_pair(t, edge)] = 1.0f;///edge->tile()->get_cost();
			}
		}
		//profile::manager pman1("create_graph_from_map - xxx");
		pathfinding::DirectedGraph<const hex_object*>::Pointer dg = std::make_shared<pathfinding::DirectedGraph<const hex_object*>>(&vertices, &edges);
		return std::make_shared<pathfinding::WeightedDirectedGraph<const hex_object*,float>>(dg, &weights);
	}

	std::vector<const hex_object*> cost_search(hex_graph_ptr graph, const hex_object* src, float max_cost)
	{
		profile::manager pman("cost_search");
		return pathfinding::path_cost_search<const hex_object*,float>(graph, src, max_cost);
	}

	void explore(std::vector<const hex_object*>* res, hex_map_ptr map, std::vector<const hex_object*>& tiles, float cost, float max_cost)
	{
		for(auto& t : tiles) {
			float t_cost = cost + t->tile()->get_cost();
			if(t_cost <= max_cost) {
				res->emplace_back(t);
				explore(res, map, map->get_surrounding_tiles(t->x(), t->y()), t_cost, max_cost);
			}
		}
	}

	// Quick and simple function to finds all moves up to and including a cost of max_cost.
	std::vector<const hex_object*> find_available_moves(hex_map_ptr map, const hex_object* src, float max_cost)
	{
		profile::manager pman("find_available_moves");
		std::vector<const hex_object*> res;
		std::deque<const hex_object*> exploration;
		exploration.emplace_back(src);

		explore(&res, map, map->get_surrounding_tiles(src->x(), src->y()), 0.0f, max_cost);

		return res;
	}
}
