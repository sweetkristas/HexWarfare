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

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>

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
	/*hex_graph_ptr create_graph_from_map(hex_map_ptr map)
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

	result_list cost_search(hex_graph_ptr graph, const hex_object* src, float max_cost)
	{
		profile::manager pman("cost_search");
		return pathfinding::path_cost_search<const hex_object*,float>(graph, src, max_cost);
	}*/

	/*result_list find_path(hex_graph_ptr graph, const hex_object* src, const hex_object* dst)
	{
		profile::manager pman("find_path");
		if(src == nullptr || dst == nullptr) {
			std::cerr << "WARN: find_path() called with nullptr. " << intptr_t(src) << " : " << intptr_t(dst) << "\n";
			return result_list();
		}
		return pathfinding::a_star_search<const hex_object*,float>(graph, src, dst, [](const hex_object* n1, const hex_object* n2){
			return (abs(n1->x() - n2->x()) + abs(n1->y() - n2->y()) + abs(n1->x() + n1->y() - n2->x() - n2->y())) / 2.0f;
		});
	}*/

	// Quick and simple function to finds all moves up to and including a cost of max_cost.
	/*std::tuple<result_list,hex_graph_ptr> find_available_moves(const engine& eng, hex_map_ptr map, const hex_object* src, float max_cost)
	{
		profile::manager pman("find_available_moves");
		
		using namespace component;
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);

		std::vector<const hex_object*> res;
		int max_area = static_cast<int>(max_cost*4.0f+1.0f);
		int x = src->x() - max_area/2;
		int w = max_area;
		if(x < 0) {
			w = w + x;
			x = 0;
		} else if(x >= static_cast<int>(map->width())) {
			w = w - (x - map->width() - 1);
			x = map->width() - 1;
		}
		int y = src->y() - max_area/2;
		int h = max_area;
		if(y < 0) {
			h = h + y;
			y = 0;
		} else if(y >= static_cast<int>(map->height())) {
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

		auto it = enemy_units.find(point(src->x(), src->y()));
		ASSERT_LOG(it == enemy_units.end(), "src node in enemies list.");

		// find vertices and edges to construct the graph.
		vertices.reserve(w*h);
		for(int m = y; m != y+h; ++m) {
			for(int n = x; n != x+w; ++n) {
				auto t = map->get_tile_at(n, m);
				auto surrounds = map->get_surrounding_tiles(t->x(), t->y());
				// scan through entities for units at t
				auto it = enemy_units.find(point(t->x(), t->y()));
				const bool in_zoc = surrounding_positions.find(point(t->x(), t->y())) != surrounding_positions.end();
				if(it == enemy_units.end()) {
					vertices.emplace_back(t);
					std::vector<const hex_object*> list_of_surrounding_tiles;
					list_of_surrounding_tiles.reserve(surrounds.size());
					for(auto& edge : surrounds) {
						if(edge->x() >= x && edge->x() < x+w 
							&& edge->y() >= y && edge->y() < y+h
							&& enemy_units.find(point(edge->x(),edge->y())) == enemy_units.end()) {
							if(!(in_zoc && surrounding_positions.find(point(edge->x(), edge->y())) != surrounding_positions.end())) {
								weights[edge_pair(t, edge)] = edge->tile()->get_cost();
								list_of_surrounding_tiles.emplace_back(edge);
							}
						}
					}
					edges[t] = list_of_surrounding_tiles;
				}
			}
		}

		// path cost search
		auto dg = std::make_shared<pathfinding::DirectedGraph<const hex_object*>>(&vertices, &edges);
		auto wdg = std::make_shared<pathfinding::WeightedDirectedGraph<const hex_object*,float>>(dg, &weights);
		res = pathfinding::path_cost_search<const hex_object*>(wdg, src, max_cost);

		// remove tiles that have friendly entities on them, from the results.
		res.erase(std::remove_if(res.begin(), res.end(), [&friendly_units](const hex_object* t){ 
			return friendly_units.find(point(t->x(), t->y())) != friendly_units.end();
		}), res.end());

		return std::make_tuple(res, wdg);
	}*/

	std::tuple<result_list,hex_graph_ptr> find_available_moves(const engine& eng, hex_map_ptr map, const hex_object* src, float max_cost)
	{
		profile::manager pman("find_available_moves");
		
		using namespace component;
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);

		std::vector<const hex_object*> res;
		int max_area = static_cast<int>(max_cost*4.0f+1.0f);
		int x = src->x() - max_area/2;
		int w = max_area;
		if(x < 0) {
			w = w + x;
			x = 0;
		} else if(x >= static_cast<int>(map->width())) {
			w = w - (x - map->width() - 1);
			x = map->width() - 1;
		}
		int y = src->y() - max_area/2;
		int h = max_area;
		if(y < 0) {
			h = h + y;
			y = 0;
		} else if(y >= static_cast<int>(map->height())) {
			h = h - (y - map->height() - 1);
			y = map->height() - 1;
		}

		//vertex_list vertices;
		std::vector<const hex_object*> vertices;
		//edge_list edges;
		//edge_weights weights;
		std::vector<edge> edges;
		std::vector<cost> weights;

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

		auto it = enemy_units.find(point(src->x(), src->y()));
		ASSERT_LOG(it == enemy_units.end(), "src node in enemies list.");

		std::map<const hex_object*, int> reverse_map;

		// find vertices and edges to construct the graph.
		vertices.reserve(w*h);
		for(int m = y; m != y+h; ++m) {
			for(int n = x; n != x+w; ++n) {
				auto n1 = map->get_tile_at(n, m);
				auto surrounds = map->get_surrounding_tiles(n1->x(), n1->y());
				// scan through entities for units at t
				auto it = enemy_units.find(point(n1->x(), n1->y()));
				const bool in_zoc = surrounding_positions.find(point(n1->x(), n1->y())) != surrounding_positions.end();
				if(it == enemy_units.end()) {
					vertices.emplace_back(n1);
					reverse_map[n1] = vertices.size()-1;
					for(auto& n2 : surrounds) {
						if(n2->x() >= x && n2->x() < x+w 
							&& n2->y() >= y && n2->y() < y+h
							&& enemy_units.find(point(n2->x(),n2->y())) == enemy_units.end()) {
							if(!(in_zoc && surrounding_positions.find(point(n2->x(), n2->y())) != surrounding_positions.end())) {
								edges.emplace_back(n1, n2);
								weights.emplace_back(n2->tile()->get_cost());
							}
						}
					}
				}
			}
		}

		hex_graph_ptr graph = std::make_shared<graph_t>(vertices.size());
		graph->reverse_map.swap(reverse_map);
		graph->vertices.swap(vertices);
		WeightMap weightmap = boost::get(boost::edge_weight, graph->graph);
		size_t n = 0;
		for(auto& ep : edges) {
			edge_descriptor e;
			bool inserted;
			boost::tie(e, inserted) = boost::add_edge(graph->reverse_map[ep.first], graph->reverse_map[ep.second], graph->graph);
			weightmap[e] = weights[n++];
		}

		std::vector<cost> d(boost::num_vertices(graph->graph));
		std::vector<vertex> p(boost::num_vertices(graph->graph));
		boost::dijkstra_shortest_paths(graph->graph, graph->reverse_map[src], 
			boost::predecessor_map(boost::make_iterator_property_map(p.begin(), boost::get(boost::vertex_index, graph->graph)))
				.distance_map(boost::make_iterator_property_map(d.begin(), boost::get(boost::vertex_index, graph->graph))));
		
		/*boost::graph_traits<hex_graph>::edge_iterator ei, ei_end;
		for(boost::tie(ei, ei_end) = boost::edges(*g); ei != ei_end; ++ei) {
			boost::graph_traits<hex_graph>::edge_descriptor e = *ei;
			boost::graph_traits<hex_graph>::vertex_descriptor u = boost::source(e, *g), v = boost::target(e, *g);
		}*/

		boost::graph_traits<hex_graph>::vertex_iterator vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph->graph); vi != vend; ++vi) {
			if(d[*vi] < max_cost) {
				res.emplace_back(graph->vertices[*vi]);
			}
		}

		// remove tiles that have friendly entities on them, from the results.
		//res.erase(std::remove_if(res.begin(), res.end(), [&friendly_units](const hex_object* t){ 
		//	return friendly_units.find(point(t->x(), t->y())) != friendly_units.end();
		//}), res.end());

		return std::make_tuple(res, graph);
	}


	struct found_goal {}; // exception for termination

	// visitor that terminates when we find the goal
	template <class Vertex>
	class astar_goal_visitor : public boost::default_astar_visitor
	{
	public:
		astar_goal_visitor(Vertex goal) : goal_(goal) {}
		template <class Graph> void examine_vertex(Vertex u, Graph& g) {
			if(u == goal_) {
				throw found_goal();
			}
		}
	private:
		Vertex goal_;
	};

	// euclidean distance heuristic
	template <class Graph, class CostType, class LocMap>
	class distance_heuristic : public astar_heuristic<Graph, CostType>
	{
	public:
		typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
		distance_heuristic(LocMap l, Vertex goal)
			: location_(l), goal_(goal) {}
		CostType operator()(Vertex u)
		{
			CostType dx = location_[goal_].x - location_[u].x;
			CostType dy = location_[goal_].y - location_[u].y;
			return ::sqrt(dx * dx + dy * dy);
		}
	private:
		LocMap location_;
		Vertex goal_;
	};

	result_list find_path(hex_graph_ptr graph, const hex_object* src, const hex_object* dst)
	{
		profile::manager pman("find_path");
		if(src == nullptr || dst == nullptr) {
			std::cerr << "WARN: find_path() called with nullptr. " << intptr_t(src) << " : " << intptr_t(dst) << "\n";
			return result_list();
		}

		auto src_it = graph->reverse_map.find(src);
		ASSERT_LOG(src_it != graph->reverse_map.end(), "source node not in graph.");
		auto dst_it = graph->reverse_map.find(dst);
		ASSERT_LOG(dst_it != graph->reverse_map.end(), "destination node not in graph.");

		std::vector<vertex> p(boost::num_vertices(graph->graph));
		std::vector<cost> d(boost::num_vertices(graph->graph));
		try {
			boost::astar_search_tree(graph->graph, *src_it, distance_heuristic<hex_graph_ptr, cost, location*>(locations, *dst_it), 
				boost::predecessor_map(boost::make_iterator_property_map(p.begin(), boost::get(boost::vertex_index, graph->graph))).
				distance_map(boost::make_iterator_property_map(d.begin(), boost::get(boost::vertex_index, graph->graph))).
				visitor(astar_goal_visitor<int>(*dst_it)));
		} catch(found_goal fg) {
			std::vector<const hex_object*> shortest_path;
			for(auto v = *dst_it;; v = p[v]) {
				shortest_path.emplace_back(graph->vertices[v]);
				if(p[v] == v) {
					return shortest_path;
				}
			}
		}
		return result_list();
	}
}
