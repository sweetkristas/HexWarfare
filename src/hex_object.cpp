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
	namespace 
	{
		std::map<std::string, tile_type_ptr>& get_tile_type_map()
		{
			static std::map<std::string, tile_type_ptr> tile_map;
			return tile_map;
		}

		std::vector<tile_type_ptr>& get_hex_editor_tiles()
		{
			static std::vector<tile_type_ptr> tiles;
			return tiles;
		}

		std::map<std::string, tile_type_ptr>& get_editor_hex_tile_map()
		{
			static std::map<std::string, tile_type_ptr> tile_map;
			return tile_map;
		}

		void load_editor_tiles()
		{
			std::map<std::string, tile_type_ptr>::const_iterator it = get_tile_type_map().begin();
			while(it != get_tile_type_map().end()) {
				if(it->second->get_editor_info().name.empty() == false 
					&& it->second->get_editor_info().type.empty() == false) {
					get_hex_editor_tiles().push_back(it->second);
				}
				++it;
			}
		}

		void load_hex_editor_tiles()
		{
			std::map<std::string, tile_type_ptr>::const_iterator it = get_tile_type_map().begin();
			while(it != get_tile_type_map().end()) {
				if(it->second->get_editor_info().type.empty() == false) {
					get_editor_hex_tile_map()[it->second->get_editor_info().type] = it->second;
				}
				++it;
			}
		}

		void load_hex_tiles(const node& n)
		{
			if(!get_tile_type_map().empty()) {
				get_tile_type_map().clear();
			}
			for(auto p : n.as_map()) {
				std::string key_str = p.first.as_string();
				get_tile_type_map()[key_str] = tile_type_ptr(new tile_type(key_str, p.second));
			}

			// get list of all tiles have non-empty "editor_info" blocks.
			if(!get_hex_editor_tiles().empty()) {
				get_hex_editor_tiles().clear();
			}
			load_editor_tiles();

			if(!get_editor_hex_tile_map().empty()) {
				get_editor_hex_tile_map().clear();
			}
			load_hex_editor_tiles();
		}

		struct hex_engine
		{
			hex_engine();
			explicit hex_engine(const node& n) {
				node tiles_var = n["tiles"];
				ASSERT_LOG(tiles_var.is_map(), "\"tiles\" must be a map type.");
				load_hex_tiles(tiles_var);
			}
		};

		hex_engine& generate_hex_engine()
		{
			static hex_engine res(json::parse_from_file("data/hex_tiles.cfg"));
			return res;
		}
	}

	void loader()
	{
		try {
			generate_hex_engine();
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/hex_tiles.cfg: " << pe.what());
		}
	}

	hex_object::hex_object(const std::string& type, int x, int y, std::weak_ptr<const hex_map> owner) 
		: owner_map_(owner), x_(x), y_(y), type_(type)
	{
		tile_ = get_tile_type_map()[type_];
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
		if(tile_ == NULL) {
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
				NeighborType* neighbor = NULL;
				for(NeighborType& candidate : neighbors_) {
					neighbor = &candidate;
				}

				if(!neighbor) {
					neighbors_.push_back(NeighborType());
					neighbor = &neighbors_.back();
					neighbor->type = obj->tile();
				}

				neighbor->dirmap = neighbor->dirmap | (1 << n);
			}
		}

		for (auto& neighbor : neighbors_) {
			neighbor.type->calculate_adjacency_pattern(neighbor.dirmap);
		}
	}

	std::vector<tile_type_ptr> hex_object::get_hex_tiles()
	{
		std::vector<tile_type_ptr> v;
		std::transform(get_tile_type_map().begin(), get_tile_type_map().end(), 
			std::back_inserter(v), 
			std::bind(&std::map<std::string, tile_type_ptr>::value_type::second,std::placeholders::_1));
		return v;
	}

	std::vector<tile_type_ptr>& hex_object::get_editor_tiles()
	{
		return get_hex_editor_tiles();
	}

	tile_type_ptr hex_object::get_hex_tile(const std::string& type)
	{
		auto it = get_editor_hex_tile_map().find(type);
		if(it == get_editor_hex_tile_map().end()) {
			it = get_tile_type_map().find(type);
			if(it == get_tile_type_map().end()) {
				return tile_type_ptr();
			}
		}
		return it->second;
	}
}
