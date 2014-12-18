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

#include "asserts.hpp"
#include "hex_logical_tiles.hpp"

namespace hex 
{
	namespace logical
	{
		namespace
		{
			typedef std::map<std::string, tile_ptr> tile_mapping_t;
			tile_mapping_t& get_loaded_tiles()
			{
				static tile_mapping_t res;
				return res;
			}
		}

		void loader(const node& n)
		{
			get_loaded_tiles().clear();

			auto& tiles = n["tiles"];
			for(auto& p : tiles.as_map()) {
				std::string id = p.first.as_string();
				float cost = p.second["cost"].as_float(1.0f);
				float height = p.second["height"].as_float(1.0f);
				std::string name = p.second["name"].as_string();
				get_loaded_tiles()[id] = std::make_shared<tile>(id, name, cost, height);
			}
		}

		tile::tile(const std::string& id, const std::string& name, float cost, float height) 
			: name_(name),
			  id_(id), 
			  cost_(cost), 
			  height_(height)
		{
		}

		tile_ptr tile::factory(const std::string& name)
		{
			auto it = get_loaded_tiles().find(name);
			ASSERT_LOG(it != get_loaded_tiles().end(), "Unable to find a tile with name: " << name);
			return it->second;
		}

		map_ptr map::factory(const node& n)
		{
			return std::make_shared<map>(n);
		}

		map::map(const node& n)
			: x_(n["x"].as_int32(0)),
		      y_(n["y"].as_int32(0)),
			  width_(n["width"].as_int32()), 
			  height_(0)
		{
			tiles_.reserve(width_ * width_);	// approximation
			for (auto& tile_str : n["tiles"].as_list_strings()) {
				tiles_.emplace_back(tile::factory(tile_str));
			}
			height_ = tiles_.size() / width_;
		}

		const_tile_ptr map::get_hex_tile(direction d, int xx, int yy) const
		{
			int ox = xx;
			int oy = yy;
			ASSERT_LOG(x() == 0 && y() == 0, "x/y values not zero (" << x() << "," << y() << ")");
			xx -= x();
			yy -= y();
			if(d == NORTH) {
				yy -= 1;
			} else if(d == SOUTH) {
				yy += 1;
			} else if(d == NORTH_WEST) {
				yy -= (abs(ox)%2==0) ? 1 : 0;
				xx -= 1;
			} else if(d == NORTH_EAST) {
				yy -= (abs(ox)%2==0) ? 1 : 0;
				xx += 1;
			} else if(d == SOUTH_WEST) {
				yy += (abs(ox)%2) ? 1 : 0;
				xx -= 1;
			} else if(d == SOUTH_EAST) {
				yy += (abs(ox)%2) ? 1 : 0;
				xx += 1;
			} else {
				ASSERT_LOG(false, "Unrecognised direction: " << d);
			}
			if (xx < 0 || yy < 0 || yy >= height() || xx >= width()) {
				return nullptr;
			}

			const int index = yy * width() + xx;
			ASSERT_LOG(index >= 0 && index < static_cast<int>(tiles_.size()), "Index out of bounds." << index << " >= " << tiles_.size());
			return tiles_[index];
		}

		point map::get_coordinates_in_dir(direction d, int xx, int yy) const
		{
			int ox = xx;
			int oy = yy;
			xx -= x();
			yy -= y();
			switch (d) {
				case NORTH:			yy -= 1; break;
				case NORTH_EAST:
					yy -= (abs(ox)%2==0) ? 1 : 0;
					xx += 1;
					break;
				case SOUTH_EAST:
					yy += (abs(ox)%2) ? 1 : 0;
					xx += 1;
					break;
				case SOUTH:			yy += 1; break;
				case SOUTH_WEST:
					yy += (abs(ox)%2) ? 1 : 0;
					xx -= 1;
					break;
				case NORTH_WEST:
					yy -= (abs(ox)%2==0) ? 1 : 0;
					xx -= 1;
					break;
				default:
					ASSERT_LOG(false, "Unrecognised direction: " << d);
					break;
			}
			return point(xx, yy) + point(x(), y());
		}

		std::vector<const_tile_ptr> map::get_surrounding_tiles(int x, int y) const
		{
			std::vector<const_tile_ptr> res;
			for(auto dir : { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST }) {
				auto hp = get_hex_tile(dir, x, y);
				if(hp != nullptr) {
					res.emplace_back(hp);
				}
			}
			return res;
		}

		std::vector<point> map::get_surrounding_positions(int xx, int yy) const
		{
			std::vector<point> res;
			for(auto dir : { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST }) {
				auto p = get_coordinates_in_dir(dir, xx, yy);
				if(p.x >= 0 && p.x >= 0 && p.x < width() && p.y < height()) {
					res.emplace_back(p);
				}
			}
			return res;
		}

		std::vector<point> map::get_surrounding_positions(const point& p) const
		{
			return get_surrounding_positions(p.x, p.y);
		}

		const_tile_ptr map::get_tile_at(int xx, int yy) const
		{
			xx -= x();
			yy -= y();
			if (xx < 0 || yy < 0 || yy >= height() || xx >= width()) {
				return nullptr;
			}

			const int index = yy * width() + xx;
			ASSERT_LOG(index >= 0 && index < static_cast<int>(tiles_.size()), "");
			return tiles_[index];
		}

		const_tile_ptr map::get_tile_at(const point& p) const
		{
			return get_tile_at(p.x, p.y);
		}

	}
}
