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

#include "castles.hpp"
#include "geometry.hpp"
#include "hex_fwd.hpp"
#include "hex_logical_tiles.hpp"
#include "hex_object.hpp"
#include "node.hpp"

namespace hex 
{
	class hex_map : public std::enable_shared_from_this<hex_map>
	{
	public:
		hex_map() : zorder_(-1000) {}
		explicit hex_map(const node& n);
		int zorder() const { return zorder_; }
		void set_zorder(int zorder) { zorder_ = zorder; }

		int x() const { return map_->x(); }
		int y() const { return map_->y(); }

		size_t width() const { return map_->width(); }
		size_t height() const { return map_->height(); }
		size_t size() const { return map_->width() * map_->height(); }
		void build();
		virtual void draw(const rect& r, const point& cam) const;
		node write() const;

		bool set_tile(int x, int y, const std::string& tile);

		std::vector<const hex_object*> get_surrounding_tiles(int x, int y) const;
		const hex_object* get_hex_tile(direction d, int x, int y) const;
		const hex_object* get_tile_at(int x, int y) const;
		const hex_object* get_tile_from_pixel_pos(int x, int y) const;
		static point get_tile_pos_from_pixel_pos(int x, int y);
		static point get_pixel_pos_from_tile_pos(int x, int y);
		static point get_pixel_pos_from_tile_pos(const point& p);

		static point loc_in_dir(int x, int y, direction d);
		static point loc_in_dir(int x, int y, const std::string& s);

		// this is a convenience function.
		logical::map_ptr get_logical_map() { return map_; }
		
		static hex_map_ptr factory(logical::map_ptr m, const node& n, const rectf& screen_area);
	private:
		logical::map_ptr map_;
		int zorder_;
		int border_;
		rectf screen_area_;
		std::vector<castle::castle_ptr> castles_;
		std::vector<hex_object> tiles_;

		hex_map(const hex_map&);
		void operator=(const hex_map&);
	};
}
