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

#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "geometry.hpp"
#include "node.hpp"

namespace pathfinding 
{
	template<typename N>
	struct PathfindingException 
	{
		const char* msg;
		const N src;
		const N dest;
	};

	template<typename N, typename T>
	class GraphNode {
	public:
		typedef std::shared_ptr<GraphNode<N, T>> GraphNodePtr;
		typedef std::weak_ptr<GraphNode<N, T>> GraphNodeWeakPtr;
		GraphNode(const N& src) 
			: f_(T(0)), 
			  g_(T(0)), 
			  h_(T(0)), 
			  src_(src), 
			  on_open_list_(false), 
			  on_closed_list_(false)
		{}
		GraphNode(const N& src, T g, T h, GraphNodeWeakPtr parent) 
			: f_(g+h), 
			  g_(g), 
			  h_(h), 
			  src_(src), 
			  parent_(parent),
			  on_open_list_(false), 
			  on_closed_list_(false)
		{}
		bool operator<(const GraphNode& rhs) const { return f_ < rhs.f_; }
		const N& getNodeValue() const { return src_; }
		T F() const { return f_; }
		T G() const { return g_; }
		T H() const { return h_; }
		void G(T g) { f_ += g - g_; g = g_; }
		void H(T h) { f_ += h - h_; h = h_; }
		void setCost(T g, T h) {
			g_ = g;
			h_ = h;
			f_ = g+h;
		}
		void setParent(GraphNodeWeakPtr parent) { parent_ = parent; }
		GraphNodePtr getParent() const { return parent_.lock(); }
		void setOnOpenList(const bool val) { on_open_list_ = val; }
		bool isOnOpenList() const { return on_open_list_;}
		void setOnClosedList(const bool val) { on_closed_list_ = val; }
		bool isOnClosedList() const { return on_closed_list_; }
		void resetNode() {
			on_open_list_ = on_closed_list_ = false;
			f_ = T(0);
			g_ = T(0);
			h_ = T(0);
			parent_.reset();
		}
	private:
		T f_, g_, h_;
		N src_;
		GraphNodeWeakPtr parent_;
		bool on_open_list_;
		bool on_closed_list_;
	};

	template<typename N, typename T> inline 
	std::ostream& operator<<(std::ostream& out, const GraphNode<N,T>& n) {
		out << "GNODE: " << n.getNodeValue() << " : cost( " << n.F() << "," << n.G() << "," << n.H() 
			<< ") : parent(" << (n.getParent() == nullptr ? "NULL" : n.getParent()->getNodeValue())
			<< ") : (" << n.isOnOpenList() << "," << n.isOnClosedList() << ")" << std::endl;
		return out;
	}

	template<typename N, typename T>
	bool graph_node_cmp(const typename GraphNode<N,T>::graph_node_ptr& lhs, 
		const typename GraphNode<N,T>::GraphNodePtr& rhs);

	template<typename N, typename T> T manhattan_distance(const N& p1, const N& p2);

	template<typename N>
	class DirectedGraph
	{
	public:
		typedef std::map<N, std::vector<N>> GraphEdgeList;
		typedef std::shared_ptr<DirectedGraph> Pointer;

		explicit DirectedGraph(std::vector<N>* vertices, GraphEdgeList* edges )
		{
			// Here we pilfer the contents of vertices and the edges.
			vertices_.swap(*vertices);
			edges_.swap(*edges);
		}
		const GraphEdgeList* getEdges() const { return &edges_; }
		const std::vector<N>& getVertices() const { return vertices_; }
		std::vector<N> getEdgesFromNode(const N& node) const {
			auto e = edges_.find(node);
			if(e != edges_.end()) {
				return e->second;
			}
			return std::vector<N>();
		}
	private:
		std::vector<N> vertices_;
		GraphEdgeList edges_;
	};

	template<typename N>
	struct EdgePair {
		typedef std::size_t result_type;
		EdgePair(const N& p1, const N& p2) : e1(p1), e2(p2) {}
		bool operator==(const EdgePair<N>& e) const {
			return e.e1 == e1 && e.e2 == e2;
		}
		bool operator<(const EdgePair<N>& e) const {
			return e1 == e.e1 ? e2 < e.e2 : e1 < e.e1;
		}
		N e1;
		N e2;
	};

	template<typename N>
	struct EdgePairHash
	{
		typedef std::size_t result_type;
		result_type operator()(EdgePair<N> const& e) const {
			const result_type h1 = std::hash<N>()(e.e1);
			const result_type h2 = std::hash<N>()(e.e2);
			return h1 ^ (h2 << 1);
		}
	};

	template<typename N, typename T>
	class WeightedDirectedGraph
	{
	public:
		typedef std::shared_ptr<WeightedDirectedGraph> Pointer;
		typedef std::map<N, typename GraphNode<N, T>::GraphNodePtr> VertexList;
		typedef std::map<typename EdgePair<N>, T> EdgeWeights;

		WeightedDirectedGraph(typename DirectedGraph<N>::Pointer dg, EdgeWeights* weights) 
			: dg_(dg)
		{
			weights_.swap(*weights);
			for(auto& v : dg->getVertices()) {
				graph_node_list_[v] = std::make_shared<GraphNode<N, T>>(v);
			}
		}
		std::vector<N> getEdgesFromNode(const N& node) const {
			return dg_->getEdgesFromNode(node);
		}
		T getWeight(const N& src, const N& dest) const {
			auto w = weights_.find(EdgePair<N>(src,dest));
			if(w != weights_.end()) {
				return w->second;
			}
			PathfindingException<N> weighted_graph_error = {"Couldn't find edge weight for nodes.", src, dest};
			throw weighted_graph_error;
		}
		typename GraphNode<N, T>::GraphNodePtr getGraphNode(const N& src) {
			auto it = graph_node_list_.find(src);
			if(it != graph_node_list_.end()) {
				return it->second;
			}
			PathfindingException<N> src_not_found = {
				"weighted_directed_graph::get_graph_node() No node found having a value of ",
				src,
				N()
			};
			throw src_not_found;
		}
		void resetGraph() {
			for(auto& p : graph_node_list_) {
				p.second->resetNode();
			}
		}
	private:
		EdgeWeights weights_;
		typename DirectedGraph<N>::Pointer dg_;
		VertexList graph_node_list_;
	};

/*	variant a_star_search(WeightedDirectedGraphPtr wg, 
		const variant src_node, 
		const variant dst_node, 
		game_logic::ExpressionPtr heuristic, 
		game_logic::MapFormulaCallablePtr callable);

	variant a_star_find_path(LevelPtr lvl, const point& src, 
		const point& dst, 
		game_logic::ExpressionPtr heuristic, 
		game_logic::ExpressionPtr weight_expr, 
		game_logic::MapFormulaCallablePtr callable, 
		const int tile_size_x, 
		const int tile_size_y);
*/

	template<typename N, typename T> inline
	std::vector<N> path_cost_search(typename WeightedDirectedGraph<N,T>::Pointer wg, const N& src_node, const T& max_cost)
	{
		std::vector<N> reachable;
		std::priority_queue<typename GraphNode<N,T>::GraphNodePtr> open_list;

		bool searching = true;
		try {
			GraphNode<N,T>::GraphNodePtr current = wg->getGraphNode(src_node);
			current->setCost(T(0), T(0));
			current->setOnOpenList(true);
			open_list.push(current);

			while(searching && !open_list.empty()) {
				current = open_list.top(); open_list.pop();
				current->setOnOpenList(false);
				if(current->G() <= max_cost) {
					reachable.push_back(current->getNodeValue());
				}

				// Push lowest f node to the closed list so we don't consider it anymore.
				current->setOnClosedList(true);
				for(const N& e : wg->getEdgesFromNode(current->getNodeValue())) {
					GraphNode<N,T>::GraphNodePtr neighbour_node = wg->getGraphNode(e);
					T g_cost(wg->getWeight(current->getNodeValue(), e) + current->G());
					if(neighbour_node->isOnClosedList() || neighbour_node->isOnOpenList()) {
						if(g_cost < neighbour_node->G()) {
							neighbour_node->G(g_cost);
							neighbour_node->setParent(current);
						}
					} else {
						// not on open or closed lists.
						neighbour_node->setParent(current);
						neighbour_node->setCost(g_cost, T(0));
						if(g_cost > max_cost) {
							neighbour_node->setOnClosedList(true);
						} else {
							neighbour_node->setOnOpenList(true);
							open_list.push(neighbour_node);
						}
					}
				}
			}
		} catch (PathfindingException<N>& e) {
			std::cerr << e.msg << " " << e.src << ", " << e.dest << std::endl;
		}
		wg->resetGraph();
		return reachable;
	}
}
