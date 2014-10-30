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

#include "geometry.hpp"
#include "hex_fwd.hpp"
#include "hex_object.hpp"
#include "node.hpp"

namespace hex 
{
	class hex_map : public std::enable_shared_from_this<hex_map>
	{
	public:
		hex_map() : zorder_(-1000), width_(0), height_(0), x_(0), y_(0)
		{}
		virtual ~hex_map()
		{}
		explicit hex_map(const node& n);
		int zorder() const { return zorder_; }
		void set_zorder(int zorder) { zorder_ = zorder; }

		int x() const { return x_; }
		int y() const { return y_; }

		size_t width() const { return width_; }
		size_t height() const { return height_; }
		size_t size() const { return width_ * height_; }
		void build();
		virtual void draw() const;
		node write() const;

		bool set_tile(int x, int y, const std::string& tile);

		hex_object_ptr get_hex_tile(direction d, int x, int y) const;
		hex_object_ptr get_tile_at(int x, int y) const;
		hex_object_ptr get_tile_from_pixel_pos(int x, int y) const;
		static point get_tile_pos_from_pixel_pos(int x, int y);
		static point get_pixel_pos_from_tile_pos(int x, int y);

		static point loc_in_dir(int x, int y, direction d);
		static point loc_in_dir(int x, int y, const std::string& s);
	private:
		std::vector<hex_object_ptr> tiles_;
		int width_;
		int height_;
		int x_;
		int y_;
		int zorder_;
	};
}
