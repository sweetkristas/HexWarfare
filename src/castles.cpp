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

#include <tuple>

#include "asserts.hpp"
#include "castles.hpp"
#include "enum_iterator.hpp"
#include "hex_fwd.hpp"
#include "json.hpp"
#include "surface.hpp"
#include "texpack.hpp"

namespace castle
{
	namespace
	{
		enum class Hexant
		{
			TOP_LEFT,
			TOP_RIGHT,
			RIGHT,
			BOTTOM_RIGHT,
			BOTTOM_LEFT,
			LEFT,
			
			First = TOP_LEFT,
			Last = LEFT,
		};

		enum class Curvature
		{
			CONVEX,
			CONCAVE,
		};

		struct tile_key
		{
			Hexant h;
			Curvature c;
			std::string name;
			typedef std::size_t result_type;
			bool operator<(const tile_key& rhs) const {
				return h == rhs.h ? c == rhs.c ? name < rhs.name : c < rhs.c : h < rhs.h;
			}
		};

		typedef std::map<tile_key, tile> tile_map;

		tile_map& get_tile_map()
		{
			static tile_map res;
			return res;
		}

		Hexant get_hexant_from_string(const std::string& s) 
		{
			if(s == "topleft" || s == "tl" || s == "TL" || s == "top-left") {
				return Hexant::TOP_LEFT;
			} else if(s == "topright" || s == "tr" || s == "TR" || s == "top-right") {
				return Hexant::TOP_RIGHT;
			} else if(s == "bottomleft" || s == "bl" || s == "BL" || s == "bottom-left") {
				return Hexant::BOTTOM_LEFT;
			} else if(s == "bottomright" || s == "br" || s == "BR" || s == "bottom-right") {
				return Hexant::BOTTOM_RIGHT;
			} else if(s == "left" || s == "l" || s == "L") {
				return Hexant::LEFT;
			} else if(s == "right" || s == "r" || s == "R") {
				return Hexant::RIGHT;
			}
			ASSERT_LOG(false, "Unrecognised hexant: " << s);
			return Hexant::TOP_LEFT;
		}

		const std::string& get_hexant_string(Hexant h) 
		{
			static std::vector<std::string> hexants;
			if(hexants.empty()) {
				hexants.emplace_back("top-left");
				hexants.emplace_back("top-right");
				hexants.emplace_back("right");
				hexants.emplace_back("bottom-right");
				hexants.emplace_back("bottom-left");
				hexants.emplace_back("left");
			}
			return hexants[static_cast<std::vector<std::string>::size_type>(h)];
		}

		tile_key decode_name_string(const std::string& key)
		{
			tile_key tkey;
			auto it = key.find('|');
			ASSERT_LOG(it != std::string::npos, "Internal error no first '|' found.");
			tkey.name = key.substr(0, it);
			auto it2 = key.find('|', it+1);
			ASSERT_LOG(it2 != std::string::npos, "Internal error no second '|' found.");
			std::string curve = key.substr(it+1, it2-it-1);
			if(curve == "convex") {
				tkey.c = Curvature::CONVEX;
			} else if(curve == "concave") {
				tkey.c = Curvature::CONCAVE;
			} else {
				ASSERT_LOG(false, "Internal error: curvature is wrong: " << curve);
			}
			tkey.h = get_hexant_from_string(key.substr(it2+1));
			return tkey;
		}

		point get_offset_in_direction(hex::direction d) {
			switch(d) {
				case hex::direction::NORTH:			return point(0,-1);
				case hex::direction::NORTH_EAST:	return point(1,0);
				case hex::direction::SOUTH_EAST:	return point(1,1);
				case hex::direction::SOUTH:			return point(0,1);
				case hex::direction::SOUTH_WEST:	return point(-1,1);
				case hex::direction::NORTH_WEST:	return point(-1,0);
			}
			return point(0,0);
		}

		std::tuple<point,point> get_offsets_in_hexant(Hexant h)
		{
			switch(h)
			{
				case Hexant::TOP_LEFT:		
					return std::make_tuple(get_offset_in_direction(hex::direction::NORTH_WEST), get_offset_in_direction(hex::direction::NORTH));
				case Hexant::TOP_RIGHT:
					return std::make_tuple(get_offset_in_direction(hex::direction::NORTH), get_offset_in_direction(hex::direction::NORTH_EAST));
				case Hexant::RIGHT:
					return std::make_tuple(get_offset_in_direction(hex::direction::NORTH_EAST), get_offset_in_direction(hex::direction::SOUTH_EAST));
				case Hexant::BOTTOM_RIGHT:
					return std::make_tuple(get_offset_in_direction(hex::direction::SOUTH_EAST), get_offset_in_direction(hex::direction::SOUTH));
				case Hexant::BOTTOM_LEFT:
					return std::make_tuple(get_offset_in_direction(hex::direction::SOUTH), get_offset_in_direction(hex::direction::SOUTH_WEST));
				case Hexant::LEFT:
					return std::make_tuple(get_offset_in_direction(hex::direction::SOUTH_WEST), get_offset_in_direction(hex::direction::NORTH_WEST));
				default: break;
			}
			ASSERT_LOG(false, "Unrecognised direction: " << static_cast<int>(h));
			return std::make_tuple<point,point>(point(),point());
		}
	}

	void loader(SDL_Renderer* renderer, const node& n)
	{
		ASSERT_LOG(n.type() == node::NODE_TYPE_MAP, "castle::loader: Node type must be map. " << n.type_as_string());
		std::vector<std::pair<std::string, surface_ptr>> surfs;
		for(auto& m : n.as_map()) {
			const std::string name = m.first.as_string();
			ASSERT_LOG(m.second.has_key("convex") && m.second.has_key("concave") && m.second.has_key("keep"), 
				"Must have 'convex', 'concave' and 'keep' keys.");
			for(auto& convex : m.second["convex"].as_map()) {
				auto hexant = get_hexant_from_string(convex.first.as_string());
				auto& hexant_str = get_hexant_string(hexant);
				std::string key = name + "|convex|" + hexant_str;
				surfs.emplace_back(std::make_pair(key, std::make_shared<graphics::surface>("images/" + convex.second.as_string())));
			}
			for(auto& concave : m.second["concave"].as_map()) {
				auto hexant = get_hexant_from_string(concave.first.as_string());
				auto& hexant_str = get_hexant_string(hexant);
				std::string key = name + "|concave|" + hexant_str;
				surfs.emplace_back(std::make_pair(key, std::make_shared<graphics::surface>("images/" + concave.second.as_string())));
			}
			for(auto& keep : m.second["keep"].as_map()) {
				// todo.
			}
		}

		SDL_RendererInfo info;
		int res = SDL_GetRendererInfo(renderer, &info);
		ASSERT_LOG(res == 0, "Failed to get renderer info: " << SDL_GetError());

		for(auto& vtex : graphics::packer(surfs, info.max_texture_width, info.max_texture_height)) {
			for(auto& tex : vtex) {
				get_tile_map()[decode_name_string(tex.get_name())] = tile(tex);
			}
		}
	}

	castle::castle(const node& value)
	{
		// tiles is a list of position
		ASSERT_LOG(value.has_key("tiles"), "No 'tiles' attribute found.");
		ASSERT_LOG(value["tiles"].is_list(), "'tiles' attribute must be a list.");
		for(auto& t : value["tiles"].as_list()) {
			ASSERT_LOG(t.is_list() && t.num_elements() == 2, "Inner elements for tiles must be two-element lists.");
			ASSERT_LOG(t[0].is_int() && t[1].is_int(), "Elements inside tiles must be integers.");
			base_positions_.emplace(point(t[0].as_int32(), t[1].as_int32()));
		}

		// Complicated bit here.
		for(auto& p : base_positions_) {
			for(auto hexant : Enum<Hexant>()) {
				point o1, o2;
				std::tie(o1,o2) = get_offsets_in_hexant(hexant);
				bool b1 = base_positions_.find(p+o1) != base_positions_.end();
				bool b2 = base_positions_.find(p+o2) != base_positions_.end();
				if(b1) {
					// There is a castle tile at b1
					// So if b2 isn't occupied it will be concave
				} else {
					// There is NOT a castle tile at b1
					// So if b2 isn't occupied this will be convex
				}
			}
			
		}
	}

	castle_ptr castle::factory(const node& value)
	{
		return std::make_shared<castle>(value);
	}

	void castle::draw(const point& p) const
	{
		// XXX draw base tile here.
		for(auto& t : tiles_) {
			t.second.texture().blit(rect(t.first - p));
		}
	}

	node castle::write() const
	{
		/// XXX 
		return node();
	}

	tile::tile()
	{
	}

	tile::tile(const graphics::texture& t)
		: tex_(t)
	{
	}
}
