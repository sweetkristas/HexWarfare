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

#include <set>
#include <vector>
#include "geometry.hpp"

template<typename T, typename R=int, unsigned MAX_OBJECTS=10, unsigned MAX_LEVELS=5>
class quadtree
{
public:
	explicit quadtree(int level, const geometry::Rect<R>& bounds) 
		: level_(level), 
		  bounding_rect_(bounds) 
	{
		nodes_.reserve(4);
	}
	void clear() {
		objects_.clear();
		for(auto& n : nodes_) {
			n.clear();
		}
		nodes_.clear();
	}
	void split() {
		const R x = bounding_rect_.x();
		const R mx = bounding_rect_.mid_x();
		const R y = bounding_rect_.y();
		const R my = bounding_rect_.mid_y();
		const R sw = bounding_rect_.w()/2;
		const R sh = bounding_rect_.h()/2;

		nodes_.emplace_back(level_+1, geometry::Rect<R>(mx,  y, sw, sh));
		nodes_.emplace_back(level_+1, geometry::Rect<R>(x,   y, sw, sh));
		nodes_.emplace_back(level_+1, geometry::Rect<R>(x,  my, sw, sh));
		nodes_.emplace_back(level_+1, geometry::Rect<R>(mx, my, sw, sh));
	}
	void insert(const T& obj, const geometry::Rect<R>& r) {
		if(!nodes_.empty()) {
			int index = get_index(r);
			if(index != -1) {
				nodes_[index].insert(obj, r);
				return;
			}
		}
		objects_.insert(std::make_pair(obj, r));
		if(objects_.size() > MAX_OBJECTS && level_ < MAX_LEVELS) {
			if(nodes_.empty()) {
				split();
			}
			auto it = objects_.begin();
			while(it != objects_.end()) {
				int index = get_index(it->second);
				if(index != -1) {
					nodes_[index].insert(it->first, it->second);
					objects_.erase(it++);
				} else {
					++it;
				}
			}
		}
	}
	void remove(const T& obj) {
		auto it = objects_.begin();
		while(it != objects_.end()) {
			if(it->first == obj) {
				objects_.erase(it++);
			} else {
				++it;
			}
		}
	}
	void get_collidable(std::vector<T>& res, const geometry::Rect<R>& r) {
		int index = get_index(r);
		if(index == -1 && !nodes_.empty()) {
			nodes_[index].get_collidable(res, r);
		}
		for(auto& obj : objects_) {
			res.emplace_back(obj.first);
		}
	}
private:
	int level_;
	geometry::Rect<R> bounding_rect_;
	std::set<std::pair<T,geometry::Rect<R>>> objects_;
	std::vector<quadtree<T, R, MAX_OBJECTS, MAX_LEVELS>> nodes_;

	int get_index(const geometry::Rect<R> r) {
		int index = -1;
		bool tq = r.y() < bounding_rect_.mid_y() && r.y2() < bounding_rect_.mid_y();
		bool bq = r.y() > bounding_rect_.mid_y();
		if(r.x() < bounding_rect_.mid_x() && r.x2() < bounding_rect_.mid_x()) {
			if(tq) {
				index = 1;
			} else if(bq) {
				index = 2;
			}
		} else if(r.x() > bounding_rect_.mid_x()) {
			if(tq) {
				index = 0;
			} else if(bq) {
				index = 3;
			}
		}
		return index;
	}
};
