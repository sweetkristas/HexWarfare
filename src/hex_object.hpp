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

#include <memory>
#include <vector>

#include "hex_fwd.hpp"
#include "hex_map.hpp"
#include "hex_tile.hpp"
#include "node.hpp"
#include "texture.hpp"

namespace hex 
{
	class hex_object 
	{
	public:
		hex_object(const std::string& type, int x, int y, std::weak_ptr<const hex_map> owner);

		void draw(const point& cam) const;
	
		void build();
		const std::string& type() const { return type_; }

		const hex_object* get_tile_in_dir(enum direction d) const;
		const hex_object* get_tile_in_dir(const std::string& s) const;

		int x() const { return x_; }
		int y() const { return y_; }

		tile_type_ptr tile() const { return tile_; }

		void init_neighbors();
		void neighbors_changed();
	private:
		// map coordinates.
		int x_;
		int y_;

		tile_type_ptr tile_;

		struct NeighborType {
			NeighborType() : dirmap(0) {}
			tile_type_ptr type;
			unsigned char dirmap;
		};

		std::vector<NeighborType> neighbors_;

		// String representing the base type of this tile.
		std::string type_;
		// raw pointer to the map that owns this.
		std::weak_ptr<const hex_map> owner_map_;
	};
}
