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
#include "formatter.hpp"
#include "hex_tile.hpp"
#include "hex_map.hpp"
#include "hex_object.hpp"
#include "json.hpp"
#include "node_utils.hpp"


namespace hex 
{
	hex_object::hex_object(const std::string& type, int x, int y, std::weak_ptr<const hex_map> owner) 
		: owner_map_(owner), x_(x), y_(y), type_(type)
	{
		tile_ = tile_type::factory(type_);
		ASSERT_LOG(tile_, "Could not find tile: " << type_);
	}

	const hex_object* hex_object::get_tile_in_dir(enum direction d) const
	{
		auto owner = owner_map_.lock();
		ASSERT_LOG(owner != nullptr, "owner_map_ was null");
		return owner->get_hex_tile(d, x_, y_);
	}

	const hex_object* hex_object::get_tile_in_dir(const std::string& s) const
	{
		if(s == "north" || s == "n") {
			return get_tile_in_dir(NORTH);
		} else if(s == "south" || s == "s") {
			return get_tile_in_dir(SOUTH);
		} else if(s == "north_west" || s == "nw" || s == "northwest") {
			return get_tile_in_dir(NORTH_WEST);
		} else if(s == "north_east" || s == "ne" || s == "northeast") {
			return get_tile_in_dir(NORTH_EAST);
		} else if(s == "south_west" || s == "sw" || s == "southwest") {
			return get_tile_in_dir(SOUTH_WEST);
		} else if(s == "south_east" || s == "se" || s == "southeast") {
			return get_tile_in_dir(SOUTH_EAST);
		}
		return nullptr;
	}

	void hex_object::draw(const point& cam) const
	{
		// Draw base tile.
		if(tile_ == nullptr) {
			return;
		}

		tile_->draw(x_, y_, cam);

		for(const NeighborType& neighbor : neighbors_) {
			neighbor.type->draw_adjacent(x_, y_, cam, neighbor.dirmap);
		}
	}

	void hex_object::neighbors_changed()
	{
		for (auto& neighbor : neighbors_) {
			neighbor.type->calculate_adjacency_pattern(neighbor.dirmap);
		}
	}

	void hex_object::init_neighbors()
	{
		for(int n = 0; n < 6; ++n) {
			const hex_object* obj = get_tile_in_dir(static_cast<direction>(n));
			if(obj && obj->tile() && obj->tile()->height() > tile()->height()) {
				NeighborType* neighbor = nullptr;
				for(NeighborType& candidate : neighbors_) {
					neighbor = &candidate;
				}

				if(neighbor == nullptr) {
					neighbors_.push_back(NeighborType());
					neighbor = &neighbors_.back();
					neighbor->type = obj->tile();
				}

				neighbor->dirmap |= (1 << n);
			}
		}
	}
}
