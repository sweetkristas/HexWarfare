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

#include <sstream>
#include <tuple>

#include "asserts.hpp"
#include "castles.hpp"
#include "enum_iterator.hpp"
#include "hex_fwd.hpp"
#include "hex_object.hpp"
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

		typedef std::map<std::string, hex::tile_type_ptr> base_texture_type;
		base_texture_type& get_base_texture() 
		{
			static base_texture_type res;
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

		Hexant opposite_hexant(Hexant h) 
		{
			switch (h)
			{
				case Hexant::TOP_LEFT:		return Hexant::BOTTOM_RIGHT;
				case Hexant::TOP_RIGHT:		return Hexant::BOTTOM_LEFT;
				case Hexant::RIGHT:			return Hexant::LEFT;
				case Hexant::BOTTOM_RIGHT:	return Hexant::TOP_LEFT;
				case Hexant::BOTTOM_LEFT:	return Hexant::TOP_RIGHT;
				case Hexant::LEFT:			return Hexant::RIGHT;
				default: break;
			}
			ASSERT_LOG(false, "Unrecognised hexant value: " << static_cast<int>(h));
			return Hexant::First;
		}

		std::ostream& operator<<(std::ostream& os, const Curvature& c) 
		{
			os << (c == Curvature::CONCAVE ? "concave" : "convex");
			return os;
		}

		std::ostream& operator<<(std::ostream& os, const tile_key& tk) 
		{
			os << "('" << tk.name << "', " << tk.c << ", " << get_hexant_string(tk.h) << ")";
			return os;
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

		struct tile_info
		{
			std::string name;
			point offset;
		};
	}

	void loader(SDL_Renderer* renderer, const node& n)
	{
		ASSERT_LOG(n.type() == node::NODE_TYPE_MAP, "castle::loader: Node type must be map. " << n.type_as_string());
		std::vector<std::pair<tile_info, surface_ptr>> surfs;
		for(auto& m : n.as_map()) {
			point offset;
			if(m.second.has_key("offset") && m.second["offset"].is_list() && m.second["offset"].num_elements() == 2) {
				offset.x = m.second["offset"][0].as_int32();
				offset.y = m.second["offset"][1].as_int32();
			}
			const std::string name = m.first.as_string();
			ASSERT_LOG(m.second.has_key("convex") && m.second.has_key("concave") && m.second.has_key("keep"), 
				"Must have 'convex', 'concave' and 'keep' keys.");
			for(auto& convex : m.second["convex"].as_map()) {
				auto hexant = get_hexant_from_string(convex.first.as_string());
				auto& hexant_str = get_hexant_string(hexant);
				std::string key = name + "|convex|" + hexant_str;
				tile_info ti = { key, offset };
				surfs.emplace_back(std::make_pair(ti, std::make_shared<graphics::surface>("images/" + convex.second.as_string())));
			}
			for(auto& concave : m.second["concave"].as_map()) {
				auto hexant = get_hexant_from_string(concave.first.as_string());
				auto& hexant_str = get_hexant_string(hexant);
				std::string key = name + "|concave|" + hexant_str;
				tile_info ti = { key, offset };
				surfs.emplace_back(std::make_pair(ti, std::make_shared<graphics::surface>("images/" + concave.second.as_string())));
			}
			for(auto& keep : m.second["keep"].as_map()) {
				// todo.
			}
			ASSERT_LOG(m.second.has_key("base"), "No 'base' attribute found.");
			auto tile_ptr = hex::hex_object::get_hex_tile(m.second["base"].as_string());
			ASSERT_LOG(tile_ptr != nullptr, "No base tile found named '" << m.second["base"].as_string() << "'");
			get_base_texture()[name] = tile_ptr;
		}

		SDL_RendererInfo info;
		int res = SDL_GetRendererInfo(renderer, &info);
		ASSERT_LOG(res == 0, "Failed to get renderer info: " << SDL_GetError());

		for(auto& vtex : graphics::packer<tile_info>(surfs, info.max_texture_width, info.max_texture_height)) {
			for(auto& tex : vtex) {
				get_tile_map()[decode_name_string(tex.first.name)] = tile(tex.second, tex.first.offset);
			}
		}
	}

	castle::castle(const node& value)
	{
		ASSERT_LOG(value.has_key("type"), "castle section must 'type' attribute.");
		tile_key tk;
		tk.name = value["type"].as_string();
		hex::tile_type_ptr base_tile = get_base_texture()[tk.name];
		ASSERT_LOG(base_tile != nullptr, "Couldn't find a reference to a base tile named '" << tk.name << "'");

		// tiles is a list of position
		ASSERT_LOG(value.has_key("tiles"), "No 'tiles' attribute found.");
		ASSERT_LOG(value["tiles"].is_list(), "'tiles' attribute must be a list.");
		for(auto& t : value["tiles"].as_list()) {
			ASSERT_LOG(t.is_list() && t.num_elements() == 2, "Inner elements for tiles must be two-element lists.");
			ASSERT_LOG(t[0].is_int() && t[1].is_int(), "Elements inside tiles must be integers.");
			point p(t[0].as_int32(), t[1].as_int32());
			base_positions_.emplace(p);
			base_tiles_.emplace_back(std::make_pair(p, base_tile));
		}

		// Complicated bit here.
		for(auto& p : base_positions_) {
			for(auto hexant : Enum<Hexant>()) {
				tk.h = opposite_hexant(hexant);
				point o1, o2;
				std::tie(o1,o2) = get_offsets_in_hexant(hexant);
				bool b1 = base_positions_.find(p+o1) != base_positions_.end();
				bool b2 = base_positions_.find(p+o2) != base_positions_.end();
				int index = b1 ? 1 : 0 + b2 ? 2 : 0;
				const static bool has_convex_tile_at_t1[]  = { true, false, false, false };
				const static bool has_concave_tile_at_t1[] = { false, true, false, false };
				const static bool has_convex_tile_at_t2[]  = { true, false, false, false };
				const static bool has_concave_tile_at_t2[] = { false, false, true, false };
				if(has_convex_tile_at_t1[index]) {
					tk.c = Curvature::CONVEX;
					auto tm = get_tile_map().find(tk);
					ASSERT_LOG(tm != get_tile_map().end(), "Unable to find tile matching this key: " << tk);
					tiles_.emplace_back(std::make_pair(p+o1, tm->second));
					std::cerr << "ADDED: " << tk << " at " << (p+o1) << " : " << b1 << "/" << b2 << "\n";
				} else if(has_concave_tile_at_t1[index]) {
					tk.c = Curvature::CONCAVE;
					auto tm = get_tile_map().find(tk);
					ASSERT_LOG(tm != get_tile_map().end(), "Unable to find tile matching this key: " << tk);
					tiles_.emplace_back(std::make_pair(p+o1, tm->second));
					std::cerr << "ADDED: " << tk << " at " << (p+o1) << " : " << b1 << "/" << b2 << "\n";
				}

				if(has_convex_tile_at_t2[index]) {
					tk.c = Curvature::CONVEX;
					auto tm = get_tile_map().find(tk);
					ASSERT_LOG(tm != get_tile_map().end(), "Unable to find tile matching this key: " << tk);
					tiles_.emplace_back(std::make_pair(p+o2, tm->second));
					std::cerr << "ADDED: " << tk << " at " << (p+o2) << " : " << b1 << "/" << b2 << "\n";
				} else if(has_concave_tile_at_t2[index]) {
					tk.c = Curvature::CONCAVE;
					auto tm = get_tile_map().find(tk);
					ASSERT_LOG(tm != get_tile_map().end(), "Unable to find tile matching this key: " << tk);
					tiles_.emplace_back(std::make_pair(p+o2, tm->second));
					std::cerr << "ADDED: " << tk << " at " << (p+o2) << " : " << b1 << "/" << b2 << "\n";
				}
			}
		}
	}

	castle_ptr castle::factory(const node& value)
	{
		return std::make_shared<castle>(value);
	}

	void castle::draw(const point& cam) const
	{
		for(auto& t : base_tiles_) {
			t.second->draw(t.first.x, t.first.y, cam);
		}

		// XXX draw base tile here.
		for(auto& t : tiles_) {
			point p(hex::hex_map::get_pixel_pos_from_tile_pos(t.first.x, t.first.y));
			t.second.texture().blit(rect(p.x - cam.x - t.second.offset().x, p.y - cam.y - t.second.offset().y));
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

	tile::tile(const graphics::texture& t, const point& offset)
		: tex_(t),
		  offset_(offset)
	{
	}
}
