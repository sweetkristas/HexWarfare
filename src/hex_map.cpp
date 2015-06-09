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

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>

#include "asserts.hpp"
#include "hex_map.hpp"
#include "hex_object.hpp"
#include "hex_tile.hpp"
#include "json.hpp"
#include "node.hpp"
#include "node_utils.hpp"

namespace hex 
{
	static const int HexTileSize = 72;

	hex_map::hex_map(const node& value)
		: zorder_(value["zorder"].as_int32(-1000)),
		  border_(value["border"].as_int32(0))
	{
		
		for(auto c : value["castles"].as_map()) {
			// c.first is a name
			castles_.emplace_back(castle::castle::factory(c.second));
		}
	}

	hex_map_ptr hex_map::factory(logical::map_ptr m, const node& n, const rectf& screen_area)
	{
		hex_map_ptr p = std::make_shared<hex_map>(n);
		p->map_ = m;
		int index = 0;
		p->tiles_.reserve(p->map_->size());
		for(auto& t : *p->map_) {
			const int x = index % p->map_->width();
			const int y = index / p->map_->width();
			p->tiles_.emplace_back(t->id(), x, y, p);
			++index;
		}
		
		for(auto& t : p->tiles_) {
			t.init_neighbors();
		}
		for(auto& t : p->tiles_) {
			t.neighbors_changed();
		}
		p->screen_area_ = screen_area;
		return p;
	}

	void hex_map::draw(const rect& r, const point& cam) const
	{
		// XXX: only draw tiles that are within the area of adjust.
		// maybe should have a clip scope as well.
		rect adjust(r.x() + static_cast<int>(r.w() * screen_area_.x()),
			r.y() + static_cast<int>(r.h() * screen_area_.y()),
			static_cast<int>(r.w() * screen_area_.w()),
			static_cast<int>(r.h() * screen_area_.h()));

		for(auto& t : tiles_) {
			t.draw(cam);
		}
		for(auto& c : castles_) {
			c->draw(cam);
		}
		// Need to draw border here -- as applicable.
	}

	node hex_map::write() const
	{
		ASSERT_LOG(false, "XXX writeme hex_map::write()");
		return node();
	}

	std::vector<const hex_object*> hex_map::get_surrounding_tiles(int x, int y) const
	{
		std::vector<const hex_object*> res;
		for(auto dir : { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST }) {
			auto hp = get_hex_tile(dir, x, y);
			if(hp != nullptr) {
				res.emplace_back(hp);
			}
		}
		return res;
	}

	const hex_object* hex_map::get_hex_tile(direction d, int x, int y) const
	{
		int ox = x;
		int oy = y;
		assert(map_->x() == 0 && map_->y() == 0);
		x -= map_->x();
		y -= map_->y();
		if(d == NORTH) {
			y -= 1;
		} else if(d == SOUTH) {
			y += 1;
		} else if(d == NORTH_WEST) {
			y -= (abs(ox)%2==0) ? 1 : 0;
			x -= 1;
		} else if(d == NORTH_EAST) {
			y -= (abs(ox)%2==0) ? 1 : 0;
			x += 1;
		} else if(d == SOUTH_WEST) {
			y += (abs(ox)%2) ? 1 : 0;
			x -= 1;
		} else if(d == SOUTH_EAST) {
			y += (abs(ox)%2) ? 1 : 0;
			x += 1;
		} else {
			ASSERT_LOG(false, "Unrecognised direction: " << d);
		}
		if (x < 0 || y < 0 || y >= map_->height() || x >= map_->width()) {
			return nullptr;
		}

		const int index = y * map_->width() + x;
		assert(index >= 0 && index < static_cast<int>(tiles_.size()));
		return &tiles_[index];
	}

	point hex_map::get_tile_pos_from_pixel_pos(int mx, int my)
	{
		const int tesselation_x_size = (3 * HexTileSize) / 2;
		const int tesselation_y_size = HexTileSize;
		const int x_base = (mx>=0) ? mx / tesselation_x_size * 2 : mx / tesselation_x_size * 2 - 2;
		const int x_mod  = (mx>=0) ? mx % tesselation_x_size : tesselation_x_size + (mx % tesselation_x_size);
		const int y_base = (my>=0) ? my / tesselation_y_size : my / tesselation_y_size - 1;
		const int y_mod  = (my>=0) ? my % tesselation_y_size : tesselation_y_size + (my % tesselation_y_size);
		const int m = 2;

		int x_modifier = 0;
		int y_modifier = 0;

		if(y_mod < tesselation_y_size / 2) {
			if((x_mod * m + y_mod) < (HexTileSize / 2)) {
				x_modifier = -1;
				y_modifier = -1;
			} else if ((x_mod * m - y_mod) < (HexTileSize * 3 / 2)) {
				x_modifier = 0;
				y_modifier = 0;
			} else {
				x_modifier = 1;
				y_modifier = -1;
			}

		} else {
			if((x_mod * m - (y_mod - HexTileSize / 2)) < 0) {
				x_modifier = -1;
				y_modifier = 0;
			} else if((x_mod * m + (y_mod - HexTileSize / 2)) < HexTileSize * 2) {
				x_modifier = 0;
				y_modifier = 0;
			} else {
				x_modifier = 1;
				y_modifier = 0;
			}
		}
		return point(x_base + x_modifier, y_base + y_modifier);
	}

	const hex_object* hex_map::get_tile_from_pixel_pos(int mx, int my) const
	{
		point p = get_tile_pos_from_pixel_pos(mx, my);
		return get_tile_at(p.x, p.y);
	}

	point hex_map::get_pixel_pos_from_tile_pos(const point& p)
	{
		return get_pixel_pos_from_tile_pos(p.x, p.y);
	}

	point hex_map::get_pixel_pos_from_tile_pos(int x, int y)
	{
		const int HexTileSizeHalf = HexTileSize/2;
		const int HexTileSizeThreeQuarters = (HexTileSize*3)/4;
		const int tx = x*HexTileSizeThreeQuarters;
		const int ty = HexTileSize*y + (abs(x)%2)*HexTileSizeHalf;
		return point(tx, ty);
	}

	const hex_object* hex_map::get_tile_at(int x, int y) const
	{
		x -= map_->x();
		y -= map_->y();
		if (x < 0 || y < 0 || y >= map_->height() || x >= map_->width()) {
			return nullptr;
		}

		const int index = y * map_->width() + x;
		assert(index >= 0 && index < static_cast<int>(tiles_.size()));
		return &tiles_[index];
	}

	bool hex_map::set_tile(int xx, int yy, const std::string& tile)
	{
		if(xx < 0 || yy < 0 || xx >= map_->width() || yy >= map_->height()) {
			return false;
		}

		const int index = yy * map_->width() + xx;
		assert(index >= 0 && index < static_cast<int>(tiles_.size()));

		tiles_[index] = hex_object(tile, xx, yy, shared_from_this());
		for(auto t : tiles_) {
			t.neighbors_changed();
		}
		return true;
	}

	point hex_map::loc_in_dir(int x, int y, direction d)
	{
		int ox = x;
		int oy = y;
		if(d == NORTH) {
			y -= 1;
		} else if(d == SOUTH) {
			y += 1;
		} else if(d == NORTH_WEST) {
			y -= (abs(ox)%2==0) ? 1 : 0;
			x -= 1;
		} else if(d == NORTH_EAST) {
			y -= (abs(ox)%2==0) ? 1 : 0;
			x += 1;
		} else if(d == SOUTH_WEST) {
			y += (abs(ox)%2) ? 1 : 0;
			x -= 1;
		} else if(d == SOUTH_EAST) {
			y += (abs(ox)%2) ? 1 : 0;
			x += 1;
		} else {
			ASSERT_LOG(false, "Unrecognised direction: " << d);
		}
		return point(x, y);
	}

	point hex_map::loc_in_dir(int x, int y, const std::string& s)
	{
		if(s == "north" || s == "n") {
			return loc_in_dir(x, y, NORTH);
		} else if(s == "south" || s == "s") {
			return loc_in_dir(x, y, SOUTH);
		} else if(s == "north_west" || s == "nw" || s == "northwest") {
			return loc_in_dir(x, y, NORTH_WEST);
		} else if(s == "north_east" || s == "ne" || s == "northeast") {
			return loc_in_dir(x, y, NORTH_EAST);
		} else if(s == "south_west" || s == "sw" || s == "southwest") {
			return loc_in_dir(x, y, SOUTH_WEST);
		} else if(s == "south_east" || s == "se" || s == "southeast") {
			return loc_in_dir(x, y, SOUTH_EAST);
		}
		ASSERT_LOG(false, "Unreognised direction " << s);
		return point();
	}
}
