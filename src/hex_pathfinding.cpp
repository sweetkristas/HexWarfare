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
	hex_graph_ptr create_graph(const engine& eng, hex_map_ptr map, int x, int y, int w, int h)
	{
		profile::manager pman("create_graph");
		
		using namespace component;
		static const component_id unit_mask = genmask(Component::POSITION) | genmask(Component::CREATURE);

		std::vector<const hex_object*> vertices;
		std::vector<edge> edges;
		std::vector<cost> weights;

		if(w == 0) {
			w = map->width();
		}
		if(h == 0) {
			h = map->height();
		}

		// Scan through enemy entities add to dictionary keyed on position.
		// For each valid surrounding position of the enemy entity add this to a set of positions.
		// enemies on a tile make that tile unavailable as a desitination.
		// surrounding positions have no edges from them to other nodes. other nodes may
		// have edges to them.
		// XXX todo.

		//std::set<point> friendly_units;
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
					//friendly_units.emplace(pos);
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

		return graph;
	}

	hex_graph_ptr create_cost_graph(const engine& eng, hex_map_ptr map, int srcx, int srcy, float max_cost)
	{
		int max_area = static_cast<int>(max_cost*4.0f+1.0f);
		int x = srcx - max_area/2;
		int w = max_area;
		if(x < 0) {
			w = w + x;
			x = 0;
		} else if(x >= static_cast<int>(map->width())) {
			w = w - (x - map->width() - 1);
			x = map->width() - 1;
		}
		int y = srcy - max_area/2;
		int h = max_area;
		if(y < 0) {
			h = h + y;
			y = 0;
		} else if(y >= static_cast<int>(map->height())) {
			h = h - (y - map->height() - 1);
			y = map->height() - 1;
		}

		return create_graph(eng, map, x, y, w, h);
	}

	result_list find_available_moves(hex_graph_ptr graph, const hex_object* src, float max_cost)
	{
		profile::manager pman("find_available_moves");
		
		result_list res;

		std::vector<cost> d(boost::num_vertices(graph->graph));
		std::vector<vertex> p(boost::num_vertices(graph->graph));
		boost::dijkstra_shortest_paths(graph->graph, graph->reverse_map[src],
			boost::predecessor_map(boost::make_iterator_property_map(p.begin(), boost::get(boost::vertex_index, graph->graph)))
				.distance_map(boost::make_iterator_property_map(d.begin(), boost::get(boost::vertex_index, graph->graph))));
		
		boost::graph_traits<hex_graph>::vertex_iterator vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph->graph); vi != vend; ++vi) {
			if(d[*vi] < max_cost) {
				res.emplace_back(graph->vertices[*vi], d[*vi]);
			}
		}

		// remove tiles that have friendly entities on them, from the results.
		/*res.erase(std::remove_if(res.begin(), res.end(), [&friendly_units](const hex_object* t){ 
			return friendly_units.find(point(t->x(), t->y())) != friendly_units.end();
		}), res.end());*/

		return res;
	}


	struct found_goal {}; // exception for termination

	// visitor that terminates when we find the goal
	template<typename Vertex>
	class astar_goal_visitor : public boost::default_astar_visitor
	{
	public:
		astar_goal_visitor(Vertex goal) : goal_(goal) {}
		template<typename Graph> void examine_vertex(Vertex u, Graph& g) {
			if(u == goal_) {
				throw found_goal();
			}
		}
	private:
		Vertex goal_;
	};

	template <class Graph, class CostType>
	class astar_heuristic : public std::unary_function<typename boost::graph_traits<Graph>::vertex_descriptor, CostType>
	{
	public:
		typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
		astar_heuristic(Vertex goal, const std::vector<const hex_object*>& vertices) 
			: goal_x_(vertices[goal]->x()), 
			  goal_y_(vertices[goal]->y()), 
			  vertices_(vertices) {}
		CostType operator()(Vertex u) 
		{ 
			const auto u_x = vertices_[u]->x();
			const auto u_y = vertices_[u]->y();
			return static_cast<CostType>((abs(u_x - goal_x_) + abs(u_y - goal_y_) + abs(u_x + u_y - goal_x_ - goal_y_)) / 2.0f);
		}
	private:
		int goal_x_;
		int goal_y_;
		const std::vector<const hex_object*>& vertices_;
	};

	result_path find_path(hex_graph_ptr graph, const hex_object* src, const hex_object* dst)
	{
		profile::manager pman("find_path");
		if(src == nullptr || dst == nullptr) {
			std::cerr << "WARN: find_path() called with nullptr. " << intptr_t(src) << " : " << intptr_t(dst) << "\n";
			return result_path();
		}

		auto src_it = graph->reverse_map.find(src);
		ASSERT_LOG(src_it != graph->reverse_map.end(), "source node not in graph.");
		auto dst_it = graph->reverse_map.find(dst);
		ASSERT_LOG(dst_it != graph->reverse_map.end(), "destination node not in graph.");

		std::vector<vertex> p(boost::num_vertices(graph->graph));
		std::vector<cost> d(boost::num_vertices(graph->graph));
		try {
			boost::astar_search_tree(graph->graph, src_it->second, astar_heuristic<hex_graph, cost>(dst_it->second, graph->vertices), 
				boost::predecessor_map(boost::make_iterator_property_map(p.begin(), boost::get(boost::vertex_index, graph->graph))).
				distance_map(boost::make_iterator_property_map(d.begin(), boost::get(boost::vertex_index, graph->graph))).
				visitor(astar_goal_visitor<vertex>(dst_it->second)));
		} catch(found_goal /*fg*/) {
			result_path shortest_path;
			for(vertex v = dst_it->second;; v = p[v]) {
				shortest_path.emplace_back(graph->vertices[v]);
				if(p[v] == v) {
					std::reverse(shortest_path.begin(), shortest_path.end());
					return shortest_path;
				}
			}
		}
		return result_path();
	}
}
