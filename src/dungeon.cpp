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

#include <iterator>
#include <limits>

#include "asserts.hpp"
#include "dungeon.hpp"
#include "node_utils.hpp"
#include "profile_timer.hpp"
#include "random.hpp"
#include "texture.hpp"
#include "wm.hpp"

namespace dungeon
{
	namespace
	{
		enum class Direction
		{
			NW,
			N,
			NE,
			W,
			E,
			SW,
			S,
			SE,
			MAX,
		};

		const int match_anything_but = -1;

		const int min_dungeon_size = 16;
		const int max_dungeon_size = 64;

		const int min_room_size = 5;
		const int max_room_size = 8;

		const int any_value = 0;
		const int nil_value = 1;

		struct image
		{
			surface_ptr tex;
			int padding;
			int tile_size;
		};

		struct tile_rule
		{
			int index;
			std::vector<int> matches;
		};

		class dungeon_rule
		{
		public:
			dungeon_rule() : default_index_(-1), ascii_(0) {}
			dungeon_rule(int def, std::vector<tile_rule>* rules, char ascii) 
				: default_index_(def), 
				  ascii_(ascii)
			{
				rules_.swap(*rules);
			}
			const std::vector<tile_rule>& get_rules() const { return rules_; }
			int get_default() const { return default_index_; }
		private:
			int default_index_;
			char ascii_;
			std::vector<tile_rule> rules_;
		};

		typedef std::map<int,image> level_image_map;
		level_image_map& get_dungeon_tile_image()
		{
			static level_image_map res;
			return res;
		}

		// We should replace then next two with a bi-directional map
		// One is mostly for debug though.
		typedef std::map<std::string,int> name_index_map;
		name_index_map& get_name_index_map()
		{
			static name_index_map res;
			return res;
		}
		typedef std::map<int,std::string> index_name_map;
		index_name_map& get_index_name_map()
		{
			static index_name_map res;
			return res;
		}

		typedef std::map<int, dungeon_rule> tile_rule_map;
		tile_rule_map& get_rule_map()
		{
			static tile_rule_map res;
			return res;
		}

		typedef std::map<char, int> ascii_map_type;
		ascii_map_type& get_ascii_map()
		{
			static ascii_map_type res;
			return res;
		}
		typedef std::map<int, char> to_ascii_map_type;
		to_ascii_map_type& get_to_ascii_map()
		{
			static to_ascii_map_type res;
			return res;
		}

		int get_tile_type_from_ascii(char c)
		{
			auto it = get_ascii_map().find(c);
			ASSERT_LOG(it != get_ascii_map().end(), "Unable to find ascii symbol '" << c << "' in the list.");
			return it->second;
		}

		char get_ascii_from_tile_type(int tt)
		{
			auto it = get_to_ascii_map().find(tt);
			ASSERT_LOG(it != get_to_ascii_map().end(), "Unable to find tiletype value '" << tt << "' in the list.");
			return it->second;
		}

		// Checks that the direction is one we know
		Direction get_direction_index(const std::string& dir)
		{
			if(dir == "NW" || dir == "nw" || dir == "northwest") {
				return Direction::NW;
			} else if(dir == "N" || dir == "n" || dir == "north") {
				return Direction::N;
			} else if(dir == "NE" || dir == "ne" || dir == "northeast") {
				return Direction::NE;
			} else if(dir == "W" || dir == "w" || dir == "west") {
				return Direction::W;
			} else if(dir == "E" || dir == "e" || dir == "east") {
				return Direction::E;
			} else if(dir == "SW" || dir == "sw" || dir == "southwest") {
				return Direction::SW;
			} else if(dir == "S" || dir == "s" || dir == "south") {
				return Direction::S;
			} else if(dir == "SE" || dir == "se" || dir == "southeast") {
				return Direction::SE;
			}
			ASSERT_LOG(false, "Unrecognised direction: " << dir);
			return Direction::N;
		}

		int match_tile(int tile, const std::vector<int>& surrounds)
		{
			int max_dir = static_cast<int>(Direction::MAX);
			ASSERT_LOG(surrounds.size() == static_cast<unsigned>(max_dir), "Surrounds is wrong size: " << surrounds.size() << " != " << max_dir);
			auto it = get_rule_map().find(tile);
			ASSERT_LOG(it != get_rule_map().end(), 
				"Couldn't find a dungeon rule matching the value: " << tile);
			const auto& dr = it->second;
			auto& rules = dr.get_rules();
			
			for(auto& rule : rules) {
				bool accept = true;
				for(int n = 0; n != max_dir; ++n) {
					const int t1 = surrounds[n];
					const int t2 = rule.matches[n];
					if(t2 == any_value || t1 == t2 || (t2 < 0 && t1 != -t2)) {
						continue;
					}
					accept = false;
					break;
				}
				if(accept) {
					return rule.index;
				}
			}
			return dr.get_default();
		}

		image get_surface_from_level(int level)
		{
			auto it = get_dungeon_tile_image().find(level);
			if(it == get_dungeon_tile_image().end()) {
				// use default (level 0)
				it = get_dungeon_tile_image().find(0);
				ASSERT_LOG(it != get_dungeon_tile_image().end(), "Unable to find a default image to use for level.");
			}
			return it->second;
		}

		rect get_tile_area_from_index(const image& img, int index)
		{
			const int tiles_x = img.tex->width()  / (img.tile_size + img.padding);
			const int tiles_y = img.tex->height() / (img.tile_size + img.padding);
			const int x = (index % tiles_x) * (img.tile_size + img.padding);
			const int y = (index / tiles_x) * (img.tile_size + img.padding);
			return rect(x, y, img.tile_size, img.tile_size);
		}
	}

	// XXX This should all be folded into a dungeon_controller class.
	void load_rules(const node& v)
	{
		// Load the images to use on various levels.
		ASSERT_LOG(v.has_key("images") && v["images"].is_list(), 
			"dungeon rules file must have 'images' attribute that is a list.");
		for(unsigned n = 0; n != v["images"].num_elements(); ++n) {
			const auto& img = v["images"][n];
			ASSERT_LOG(img.is_map() && img.has_key("image") && img["image"].is_string(), 
				"elements in the images list of the dungeon rules must have 'image' attribute that is a string.");
			image image_data;
			image_data.tex = graphics::surface::create(img["image"].as_string());
			image_data.padding = img.has_key("padding") ? img["padding"].as_int32() : 0;
			image_data.tile_size = img["tile_size"].as_int32();
			if(img.has_key("levels")) {
				ASSERT_LOG(img["levels"].is_list(), "'levels' attribute must be a list.");
				for(auto &lvl : img["levels"].as_list()) {
					if(lvl.is_int()) {
						int val = lvl.as_int32();
						auto it = get_dungeon_tile_image().find(val);
						ASSERT_LOG(it == get_dungeon_tile_image().end(), "Overlapping images found in the dungeon rules file at " << val);
						get_dungeon_tile_image()[val] = image_data;
					} else if(lvl.is_list()) {
						ASSERT_LOG(lvl.num_elements() == 2, "Ranged values must have two elements.");
						int low = lvl[0].as_int32();
						int high = lvl[0].as_int32();
						for(int val = low; val <= high; ++val) {
							auto it = get_dungeon_tile_image().find(val);
							ASSERT_LOG(it == get_dungeon_tile_image().end(), "Overlapping images found in the dungeon rules file at " << val);
							get_dungeon_tile_image()[val] = image_data;
						}
					} else {
						ASSERT_LOG(false, "Unrecongnised type in levels list, must be int or list: " << lvl.type_as_string());
					}
				}
			} else {
				// Is default image
				auto it = get_dungeon_tile_image().find(0);
				ASSERT_LOG(it == get_dungeon_tile_image().end(), "Multiple default images found in the dungeon rules file.");
				get_dungeon_tile_image()[0] = image_data;
			}
		}

		// Load the ruleset
		ASSERT_LOG(v.has_key("rules") && v["rules"].is_map(), 
			"Dungeon rules file must have a 'rules' attribute that is a map.");
		auto& rule_map = v["rules"].as_map();

		// add a mapping for any value
		get_name_index_map()["any"] = any_value;
		get_index_name_map()[any_value] = "any";

		// Add a mapping for NIL(i.e. border) tiles. 
		get_name_index_map()["NIL"] = nil_value;
		get_index_name_map()[nil_value] = "NIL";

		int count = nil_value + 1;

		// Pre-scan rules to generate the names of the tiles.
		for(auto& rule : rule_map) {
			ASSERT_LOG(rule.second.has_key("default_index") && rule.second.has_key("rules"),
				"rules should have 'default_index' and 'rules' attributes.");
			const auto& name = rule.first.as_string();
			get_name_index_map()[name] = count;
			get_index_name_map()[count] = name;
			++count;
		}
		for(auto& rule : rule_map) {
			// First element is the name of the tile. Second element is the details
			const auto& name = rule.first.as_string();
			auto it = get_name_index_map().find(name);
			ASSERT_LOG(it != get_name_index_map().end(), "Something bad happened " << name << " wasn't found in our internal list.");
			int name_index = it->second;
			const int default_index = rule.second["default_index"].as_int32();
			char ascii_char = rule.second["ascii"].as_string()[0];
			auto am_it = get_ascii_map().find(ascii_char);
			ASSERT_LOG(am_it == get_ascii_map().end(), "ASCII character for '" << name << "' already used.");
			get_ascii_map()[ascii_char] = name_index;
			get_to_ascii_map()[name_index] = ascii_char;
			std::vector<tile_rule> tile_rules;
			for(auto& r : rule.second["rules"].as_list()) {
				ASSERT_LOG(r.has_key("index") && r.has_key("map"), "rules must have 'index' and 'map' attributes.");
				tile_rules.push_back(tile_rule());
				tile_rules.back().index = r["index"].as_int32();
				tile_rules.back().matches.resize(static_cast<int>(Direction::MAX), 0);
				for(auto& rmap : r["map"].as_map()) {
					auto value = rmap.second.as_string();
					bool negate = false;
					if(value[0] == '!') {
						// negation condition.
						negate = true;
						value = value.substr(1);
					}
					auto it = get_name_index_map().find(value);
					ASSERT_LOG(it != get_name_index_map().end(), "Didn't find definition for tile name: " << value);
					Direction dir = get_direction_index(rmap.first.as_string());
					tile_rules.back().matches[static_cast<int>(dir)] = it->second * (negate ? match_anything_but : 1);
				}
			}
			get_rule_map()[name_index] = dungeon_rule(default_index, &tile_rules, ascii_char);
		}
	}

	dungeon_model::dungeon_model()
	{
	}

	enum class DungeonFeature
	{
		ROOM,
		OCTAGONAL_ROOM,
		CORRIDOR,
		MAX,
	};

	// XXX all these mappings should be in a dungeon_controller class.
	// Preferably read from file.
	const char ceiling = '#';
	const char floor   = '.';
	const char pit     = 'X';
	const char lava    = '~';
	const char door    = 'D';
	const char wall    = 'T';
	class dungeon_generator
	{
	public:
		dungeon_generator(int width, int height) 
			: width_(width),
			  height_(height)
		{
			// Generate a blank map.
			nmap_.resize(height);
			for(auto& row : nmap_) {
				row.resize(width, ceiling);
			}
			
			rect r = get_random_room_size();
			r += point((width-r.w())/2, (height-r.h())/2);
			if(!place_room(r)) {
				ASSERT_LOG(false, "Couldn't place an initial room -- this is very very bad.");
			}
			profile::timer t;
			int feature_count = 0;
			while(feature_count < 10 && t.get_time() < 1000.0) {
				if(create_new_feature()) {
					++feature_count;
				}
			}
			std::cerr << "Added " << feature_count << " features in " << t.get_time() << " ms\n";
		}
		rect get_random_room_size(int x=0, int y=0) {
			int rs_w = generator::get_uniform_int<int>(min_room_size, max_room_size);
			int rs_h = generator::get_uniform_int<int>(min_room_size, max_room_size);
			return rect(x, y, rs_w, rs_h);
		}
		void fill_rect(const rect& r, char c) {
			ASSERT_LOG(r.x2() < width_ && r.x() < width_, "X Co-ordinates to fill are outside width. " << width_ << " : " << r.x() << " " << r.x2());
			ASSERT_LOG(r.y2() < height_ && r.y() < height_, "Y Co-ordinates to fill are outside height. " << height_ << " : " << r.y() << " " << r.y2());
			ASSERT_LOG(r.x() >= 0 && r.x2() >= 0, "X Co-ordinates to fill are less than zero. " << r.x() << " : " << r.x2());
			ASSERT_LOG(r.y() >= 0 && r.y2() >= 0, "Y Co-ordinates to fill are less than zero. " << r.y() << " : " << r.y2());
			for(int y = r.y(); y != r.y2(); y++) {
				for(int x = r.x(); x != r.x2(); ++x) {
					nmap_[y][x] = c;
				}
			}
		}
		bool is_rect_filled(const rect& r, char c) {
			if(r.x() < 0 || r.x2() < 0 || r.y() < 0 || r.y2() < 0 
				|| r.x() >= width_ || r.x2() >= width_
				|| r.y() >= height_ || r.y2() >= height_) {
				return false;
			}
			for(int y = 0; y != r.h(); ++y) {
				for(int x = 0; x != r.w(); ++x) {
					if(nmap_[y+r.y()][x+r.x()] != c) {
						return false;
					}					
				}
			}
			return true;
		}
		bool place_room(const rect& r) {
			if(is_rect_filled(r, ceiling)) {
				create_room(r);
				return true;
			}
			return false;
		}
		void create_room(const rect& r) {
			ASSERT_LOG(r.x2() < width_ && r.x() < width_, "Co-ordinates to fill are outside width. " << width_ << " : " << r.x() << " " << r.x2());
			ASSERT_LOG(r.y2() < height_ && r.y() < height_, "Co-ordinates to fill are outside height. " << height_ << " : " << r.y() << " " << r.y2());
			for(int y = 0; y != r.h(); ++y) {
				for(int x = 0; x != r.w(); ++x) {
					int wx = x + r.x();
					int wy = y + r.y();
					nmap_[wy][wx] = floor;
					if(x == 0 || y == 0 || x == r.w()-1 || y == r.h()-1) {
						wall_list_.emplace_back(wx, wy);
						nmap_[wy][wx] = wall;
					}
				}
			}
		}
		bool create_new_feature() {
			// Select a random feature
			DungeonFeature feature = static_cast<DungeonFeature>(generator::get_uniform_int<int>(0, static_cast<int>(DungeonFeature::MAX)-1));
			switch(feature) {
				case DungeonFeature::ROOM: return create_new_room();
				case DungeonFeature::OCTAGONAL_ROOM: break;
				case DungeonFeature::CORRIDOR: break;
				default: break;
			}
			return false;
		}
		// returns true on successfully creating a new room, false if we fail.
		bool create_new_room() {
			// choose a wall
			auto& wll = choose_wall();
			// Check it's still a wall.
			if(nmap_[wll.second][wll.first] != wall) {
				return false;
			}
			point corridor_location;
			rect rs = get_random_room_size();
			// Check each of the four cardinal directions to see if there is a free 
			// ceiling space to create a corridor
			if(wll.second > 0 && nmap_[wll.second-1][wll.first] == ceiling) {
				// north
				corridor_location = point(wll.first, wll.second-1);
				// XXX adjust rs x/y as needed here
				rs += point(wll.first - rs.w()/2, wll.second - (rs.h()+1));
			} else if(wll.second < height_-1 && nmap_[wll.second+1][wll.first] == ceiling) {
				// south
				corridor_location = point(wll.first, wll.second+1);
				// XXX adjust rs x/y as needed here
				rs += point(wll.first - rs.w()/2, wll.second+2);
			} else if(wll.first < width_-1 && nmap_[wll.second][wll.first+1] == ceiling) {
				// east
				corridor_location = point(wll.first+1, wll.second);
				// XXX adjust rs x/y as needed here
				rs += point(wll.first+2, wll.second-(rs.h()/2));
			} else if(wll.first > 0 && nmap_[wll.second][wll.first-1] == ceiling) {
				// west
				corridor_location = point(wll.first-1, wll.second);
				// XXX adjust rs x/y as needed here
				rs += point(wll.first-(rs.w()+1), wll.second-(rs.h()/2));
			} else {
				return false;
			}
			if(!place_room(rs)) {
				return false;
			}
			//create_room(rs);
			nmap_[corridor_location.y][corridor_location.x] = floor;
			return true;
		}
		const std::pair<int,int>& choose_wall() {
			return wall_list_[generator::get_uniform_int<int>(0, wall_list_.size()-1)];
		}
		const std::vector<std::vector<char>>& get_map() const { return nmap_; } 
	private:
		int width_;
		int height_;
		std::vector<std::vector<char>> nmap_;
		std::vector<std::pair<int,int>> wall_list_;
	};

	dungeon_model_ptr dungeon_model::generate(int level)
	{
		int width = generator::get_uniform_int<int>(min_dungeon_size, max_dungeon_size);
		int height = generator::get_uniform_int<int>(min_dungeon_size, max_dungeon_size);

		dungeon_generator gen(width, height);

		// Convert the ascii map to internal model
		auto dung = std::make_shared<dungeon_model>();
		dung->level_ = level;
		dung->width_ = width;
		dung->height_ = height;
		dung->tile_map_.resize(dung->height_);
		int y = 0;
		for(auto& row : dung->tile_map_) {
			int x = 0;
			row.resize(dung->width_);
			for(int x = 0; x != dung->width_; ++x) {
				row[x] = get_tile_type_from_ascii(gen.get_map()[y][x]);
			}
			++y;
		}
		return dung;
	}

	dungeon_model_ptr dungeon_model::read(const node& n)
	{
		auto dung = std::make_shared<dungeon_model>();
		ASSERT_LOG(n.has_key("level"), "Map must have a 'level' attribute.");
		ASSERT_LOG(n.has_key("map"), "Map must have a 'map' attribute.");
		dung->level_ = n["level"].as_int32();
		dung->tile_map_.resize(n["map"].num_elements());
		int y = 0;
		int row_length = std::numeric_limits<int>::min();
		for(auto& row : n["map"].as_list()) {
			const std::string& row_str = row.as_string();
			dung->tile_map_[y].resize(row_str.size());
			int x = 0;
			for(char c : row_str) {
				// XXX this should be modified to use a dungeon_controller object to hold the mapping.
				dung->tile_map_[y][x] = get_tile_type_from_ascii(c);
				++x;
			}
			if(static_cast<int>(row_str.size()) > row_length) {
				row_length = row_str.size();
			}
			++y;
		}
		dung->width_ = row_length;
		dung->height_ = dung->tile_map_.size();
		return dung;
	}

	node dungeon_model::write()
	{
		node_builder res;
		res.add("level", level_);
		for(auto& row : tile_map_) {
			std::string map_row;
			for(auto& col : row) {
				// XXX this should be modified to use a dungeon_controller object to hold the mapping.
				map_row += get_ascii_from_tile_type(col);
			}
			res.add("map", map_row);
		}			
		return res.build();
	}

	int dungeon_model::get_at(int x, int y)
	{
		//ASSERT_LOG(y < height_, "y value exceeds height of dungeon model: " << y << " >= " << height_);
		//ASSERT_LOG(y < height_, "x value exceeds width of dungeon model: " << x << " >= " << width_);
		if(y >= height_ || x >= width_ || x < 0 || y < 0) {
			return nil_value;
		}
		return tile_map_[y][x];
	}

	dungeon_view::dungeon_view(dungeon_model_ptr model)
		: model_(model)
	{
		profile::manager pman("dungeon_view constructor");

		int level = 0;

		const auto& img = get_surface_from_level(level);

		std::vector<int> surrounds(static_cast<int>(Direction::MAX));
		surface_ptr dest = graphics::surface::create(model->width()*img.tile_size, model_->height()*img.tile_size);

		for(int y = 0; y != model_->height(); ++y) {
			for(int x = 0; x != model_->width(); ++x) {
				surrounds[0] = model_->get_at(x-1, y-1);
				surrounds[1] = model_->get_at(x+0, y-1);
				surrounds[2] = model_->get_at(x+1, y-1);
				surrounds[3] = model_->get_at(x-1, y+0);
				surrounds[4] = model_->get_at(x+1, y+0);
				surrounds[5] = model_->get_at(x-1, y+1);
				surrounds[6] = model_->get_at(x+0, y+1);
				surrounds[7] = model_->get_at(x+1, y+1);
				rect area = get_tile_area_from_index(img, match_tile(model_->get_at(x+0, y+0), surrounds));
				dest->blit_scaled(img.tex, area, rect(x*img.tile_size, y*img.tile_size, img.tile_size, img.tile_size));
			}
		}
		tex_ = graphics::texture(dest, graphics::TextureFlags::NONE);
	}

	void dungeon_view::draw(int cx, int cy) const
	{
		auto& wm = graphics::window_manager::get_main_window();
		const int src_x = wm.width() > tex_.width() 
			? 0 
			: cx < wm.width()/2 
				? 0
				: cx > tex_.width() - wm.width()/2 ? tex_.width()-wm.width() : cx - wm.width()/2;
		const int src_y = wm.height() > tex_.height() 
			? 0 
			: cy < wm.height()/2 
				? 0
				: cy > tex_.height() - wm.height()/2 ? tex_.height()-wm.height() : cy - wm.height()/2;
		const int src_w = wm.width() > tex_.width() ? tex_.width() : wm.width();
		const int src_h = wm.height() > tex_.height() ? tex_.height() : wm.height();

		const int dst_x = wm.width() > tex_.width() ? (wm.width() - tex_.width())/2 : 0;
		const int dst_y = wm.height() > tex_.height() ? (wm.height() - tex_.height())/2 : 0;
		const int dst_w = wm.width() > tex_.width() ? tex_.width() : wm.width();
		const int dst_h = wm.height() > tex_.height() ? tex_.height() : wm.height();

		tex_.blit(rect(src_x, src_y, src_w, src_h), 
			rect(dst_x, dst_y, dst_w, dst_h));
	}
}
