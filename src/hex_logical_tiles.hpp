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

#include <string>
#include <vector>

#include "geometry.hpp"
#include "hex_logical_fwd.hpp"
#include "node.hpp"

namespace hex 
{
	namespace logical
	{
		std::tuple<int,int,int> oddq_to_cube_coords(const point& p);
		int distance(int x1, int y1, int z1, int x2, int y2, int z2);
		int distance(const point& p1, const point& p2);
		std::vector<point> line(const point& p1, const point& p2);

		class tile
		{
		public:
			explicit tile(const std::string& id, const std::string& name, float cost, float height);
			const std::string& name() const { return name_; }
			const std::string& id() const { return id_; }
			float get_cost() const { return cost_; }
			float get_height() const { return height_; }
			static tile_ptr factory(const std::string& name);
		private:
			std::string name_;
			std::string id_;
			float height_;
			float cost_;
		};
	
		class map
		{
		public:
			typedef std::vector<tile_ptr>::iterator iterator;
			typedef std::vector<tile_ptr>::const_iterator const_iterator;

			explicit map(const node& n);
			map_ptr clone();

			int x() const { return x_; }
			int y() const { return y_; }
			int width() const { return width_; }
			int height() const { return height_; }

			// Range based for loop support.
			iterator begin() { return tiles_.begin(); }
			iterator end() { return tiles_.end(); }
			const_iterator begin() const { return tiles_.begin(); }
			const_iterator end() const { return tiles_.end(); }
			std::size_t size() { return tiles_.size(); }

			const_tile_ptr get_hex_tile(direction d, int x, int y) const;
			std::vector<const_tile_ptr> get_surrounding_tiles(int x, int y) const;
			// Get the positions of the valid tiles surrounding the tile at (x,y)
			std::vector<point> get_surrounding_positions(int x, int y) const;
			std::vector<point> get_surrounding_positions(const point& p) const;
			const_tile_ptr get_tile_at(int xx, int yy) const;
			const_tile_ptr get_tile_at(const point& p) const;
			point get_coordinates_in_dir(direction d, int x, int y) const;

			static map_ptr factory(const node& n);
		private:
			int x_;
			int y_;
			int width_;
			int height_;

			std::vector<tile_ptr> tiles_;
			map(const map&);
		};

		void loader(const node& n);
	}
}
