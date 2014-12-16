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
	}
}
