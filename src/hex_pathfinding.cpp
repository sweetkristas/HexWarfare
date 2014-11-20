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
		profile::manager pman("create_graph_from_map");

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
				weights[edge_pair(t, edge)] = edge->tile()->get_cost();
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

	std::vector<const hex_object*> find_path(hex_graph_ptr graph, const hex_object* src, const hex_object* dst)
	{
		profile::manager pman("find_path");
		return pathfinding::a_star_search<const hex_object*,float>(graph, src, dst, [](const hex_object* n1, const hex_object* n2){
			return (abs(n1->x() - n2->x()) + abs(n1->y() - n2->y()) + abs(n1->x() + n1->y() - n2->x() - n2->y())) / 2.0f;
		});
	}

	// Quick and simple function to finds all moves up to and including a cost of max_cost.
	std::vector<const hex_object*> find_available_moves(const engine& eng, hex_map_ptr map, const hex_object* src, float max_cost)
	{
		profile::manager pman("find_available_moves");
		
		using namespace component;
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);

		std::vector<const hex_object*> res;
		int max_area = static_cast<int>(max_cost*2.0f+1.0f);
		int x = src->x() - max_area/2;
		int w = max_area;
		if(x < 0) {
			w = w + x;
			x = 0;
		} else if(x >= map->width()) {
			w = w - (x - map->width() - 1);
			x = map->width() - 1;
		}
		int y = src->y() - max_area/2;
		int h = max_area;
		if(y < 0) {
			h = h + y;
			y = 0;
		} else if(y >= map->height()) {
			h = h - (y - map->height() - 1);
			y = map->height() - 1;
		}

		vertex_list vertices;
		edge_list edges;
		edge_weights weights;

		// Scan through enemy entities add to dictionary keyed on position.
		// For each valid surrounding position of the enemy entity add this to a set of positions.
		// enemies on a tile make that tile unavailable as a desitination.
		// surrounding positions have no edges from them to other nodes. other nodes may
		// have edges to them.
		// XXX todo.

		std::set<point> friendly_units;
		std::map<point, component_set_ptr> enemy_units;
		std::set<point> surrounding_positions;
		for(auto& e : eng.get_entities()) {
			auto owner = e->owner.lock();
			if((e->mask & unit_mask) == unit_mask) {
				auto& pos = e->pos->pos;
				if(eng.get_current_player()->team() != owner->team()) {
					enemy_units[pos] = e;
					auto surrounds = map->get_surrounding_tiles(pos.x, pos.y);
					for(auto& t : surrounds) {
						surrounding_positions.emplace(t->x(), t->y());
					}
				} else {
					friendly_units.emplace(pos);
				}
			}
		}
		// remove enemy entities from surrounding positions
		for(auto& e : eng.get_entities()) {
			auto owner = e->owner.lock();
			if((e->mask & unit_mask) == unit_mask && eng.get_current_player()->team() != owner->team()) {
				auto it = surrounding_positions.find(e->pos->pos);
				if(it != surrounding_positions.end()) {
					surrounding_positions.erase(it);
				}
			}
		}

		vertices.reserve(w*h);
		for(int m = 0; m != h; ++m) {
			for(int n = 0; n != w; ++n) {
				auto t = map->get_tile_at(n, m);
				auto surrounds = map->get_surrounding_tiles(t->x(), t->y());
				// scan through entities for units at t
				auto it = enemy_units.find(point(t->x(), t->y()));
				if(it == enemy_units.end()) {
					vertices.emplace_back(t);
					std::vector<const hex_object*> list_of_surrounding_tiles(surrounds.size());
					for(auto& edge : surrounds) {
						weights[edge_pair(t, edge)] = edge->tile()->get_cost();
					}				
					edges[t] = list_of_surrounding_tiles;
				} else {
					// There should be no edges from surrounding tiles to the rest of the board.
					for(auto& surr : surrounds) {
						
					}
				}
			}
		}

		// XXX Do path cost search here

		// remove tiles that have friendly entities on them, from the results.
		res.erase(std::remove_if(res.begin(), res.end(), [&friendly_units](const hex_object* t){ 
			return friendly_units.find(point(t->x(), t->y())) == friendly_units.end();
		}), res.end());

		return res;
	}
}
