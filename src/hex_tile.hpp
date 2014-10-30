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

#include <vector>
#include <map>

#include "hex_fwd.hpp"
#include "node.hpp"
#include "texture.hpp"

namespace hex 
{
	class tile_sheet
	{
	public:
		explicit tile_sheet(const node& n);
		const graphics::texture& get_texture() const { return texture_; }
		rect get_area(int index) const;
	private:
		graphics::texture texture_;
		rect area_;
		int nrows_, ncols_, pad_;
	};

	class tile_type
	{
	public:
		tile_type(const std::string& id, const node& n);
		
		struct editor_info {
			std::string name;
			std::string type;
			graphics::texture texture;
			std::string group;
			rect image_rect;
			void draw(int tx, int ty) const;
		};

		const std::string& id() const { return id_; }

		const editor_info& get_editor_info() const { return editor_info_; } 

		const std::vector<int>& sheet_indexes() const { return sheet_indexes_; }

		void draw(int x, int y) const;

		//The lowest bit of adjmap indicates if this tile type occurs to the north
		//of the target tile, the next lowest for the north-east and so forth.
		void draw_adjacent(int x, int y, unsigned char adjmap) const;

		float height() const { return height_; }

		node write() const;
		void calculate_adjacency_pattern(unsigned char adjmap);
	private:
		std::string id_;
		tile_sheet_ptr sheet_;
		float height_;

		std::vector<int> sheet_indexes_;

		struct AdjacencyPattern {
			AdjacencyPattern() : init(false), depth(0)
			{}
			bool init;
			int depth;
			std::vector<int> sheet_indexes;
		};

		AdjacencyPattern adjacency_patterns_[64];

		editor_info editor_info_;
	};
}
