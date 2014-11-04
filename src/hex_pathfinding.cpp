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
	hex_graph_ptr create_graph_from_map(const engine& eng, const component_set_ptr& x, hex_map_ptr map)
	{
		using namespace component;
		profile::manager pman("create_graph_from_map");
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);
		player_ptr current_player = eng.get_current_player();
		auto& entities = eng.get_entities();

		vertex_list vertices;
		edge_list edges;
		edge_weights weights;

		vertices.reserve(map->get_tiles().size());
		{
		//profile::manager pman("create_graph_from_map - verts");
		// Create the vertex list and edge list.
		//vertices = map->get_tiles();
		for(const auto& tile : map->get_tiles()) {
			bool add_tile = true;
			/*for(auto entity : entities) {
				if((entity->mask & unit_mask) == unit_mask) {
					auto& pos = entity->pos->pos;
					if(pos.x == tile.x() && pos.y == tile.y()) {
						if(x != entity) {
							add_tile = false;
							break;
						}
					}
				}
			}*/
			if(add_tile) {
				vertices.emplace_back(&tile);
				auto surrounds = map->get_surrounding_tiles(tile.x(), tile.y());
				edges[&tile] = surrounds;
				for(auto& edge : surrounds) {
					// XXX replace 1.0f below with a value for the terrain being traversed to.
					//weights[edge_pair(tile, edge)] = 1.0f;
					weights.emplace(edge_pair(&tile, edge), 1.0f);
				}
			}
		}
		}
		// Remove ZoC edges
		{
		//profile::manager pman("create_graph_from_map - zoc");
		for(auto e : entities) {
			if((e->mask & unit_mask) == unit_mask) {
				auto x_owner = x->owner.lock();
				auto e_owner = e->owner.lock();
				auto& pos = e->pos->pos;
				ASSERT_LOG(x_owner != nullptr && e_owner != nullptr, "Unit entity with no owner.");
				if(x_owner->team()->id() != e_owner->team()->id()) {
					auto tiles1 = map->get_surrounding_tiles(pos.x, pos.y);
					for(auto& t1 : tiles1) {
						auto tiles2 = map->get_surrounding_tiles(t1->x(), t1->y());
						for(auto& t2 : tiles2) {
							auto it = weights.find(edge_pair(t1, t2));
							if(it != weights.end()) {
								weights.erase(it);
							}
						}
					}
				}
			}
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
}
