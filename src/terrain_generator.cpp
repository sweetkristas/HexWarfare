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

#include <algorithm>
#include <unordered_map>

#include <boost/functional/hash.hpp>
#include <noise/noise.h>
#include "noiseutils.h"

#include "asserts.hpp"
#include "engine.hpp"
#include "random.hpp"
#include "surface.hpp"
#include "terrain_generator.hpp"

namespace terrain
{
	namespace
	{
		const double terrain_scale_factor = 8.0;

		class terrain_data
		{
		public:
			terrain_data() {}
			void sort_tiles() {
				std::stable_sort(tiles_.begin(), tiles_.end());
			}
			const point& get_tile_size() const { return tile_size_; }
			void set_tile_size(const point& p) { tile_size_ = p; }
			terrain_tile_ptr get_tile_at_height(float value) const {
				if(value >= tiles_.back()->get_threshold()) {
					return tiles_.back();
				}
				for(auto& t : tiles_) {
					if(value < t->get_threshold()) {
						return t;
					}
				}
				ASSERT_LOG(false, "No valid terrain value found for value: " << value);
				return nullptr;
			}
			bool is_higher_priority(TerrainType first, TerrainType second) const {
				// is priority(first) > priority(second)
				for(const auto& priority : tile_priority_list_) {
					if(priority == first) {
						return true;
					} else if(priority == second) {
						return false;
					}
				}
				ASSERT_LOG(false, "Did not match terrain type " << first << " or " << second << " in priority list.");
				return false;
			}
			void load(const node& n) {
				ASSERT_LOG(n.has_key("tile_size") && n["tile_size"].is_list() && n["tile_size"].num_elements() == 2,
					"'tile_size' attribute in terrain data file must be a two element list.");
				ASSERT_LOG(n.has_key("tiles") && n["tiles"].is_map(),
					"terrain data must have 'tiles' attribute that is a map.");
				auto& tiles = n["tiles"].as_map();

				tiles_.resize(tiles.size());

				TerrainType counter = 0;
				set_tile_size(point(static_cast<int>(n["tile_size"][0].as_int()), static_cast<int>(n["tile_size"][1].as_int())));
				for(const auto& t : tiles) {
					const auto& name = t.first.as_string();
					ASSERT_LOG(t.second.has_key("image"), "tiles must have 'image' attribute");
					const auto& image = t.second["image"].as_string();
					ASSERT_LOG(t.second.has_key("gradient"), "tiles must have 'gradient' attribute");
					const auto& gradient = t.second["gradient"].as_float();
					std::string transitions_file;
					if(t.second.has_key("transitions")) {
						transitions_file = t.second["transitions"].as_string();
					}
					auto ntile = std::make_shared<terrain_tile>(gradient, counter, name, image, transitions_file);
					tiles_[counter] = ntile;
					tile_name_map_[ntile->get_name()] = ntile->get_terrain_type();
					++counter;
				}

				sort_tiles();

				// Load tile priorities
				// 'tile_priority' is a list of tile types of what looks better when two
				// tiles are placed next to each other.
				// e.g. jungle would look better when next to grass if the jungle encroached
				// the grass tile. i.e. provided the transitions.
				ASSERT_LOG(n.has_key("tile_priority"), "No 'tile_priority' attribute found in terrain data.");
				for(auto& s : n["tile_priority"].as_list_strings()) {
					auto it = tile_name_map_.find(s);
					ASSERT_LOG(it != tile_name_map_.end(), "Couldn't find tile named '" << s << "' in the list of terrain tiles.");
					tile_priority_list_.emplace_back(it->second);
				}
			}
			const std::vector<TerrainType>& get_priority_list() const { return tile_priority_list_; }
			terrain_tile_ptr get_tile_from_terrain(TerrainType tt) {
				ASSERT_LOG(tt >= 0 && static_cast<unsigned>(tt) < tiles_.size(), "TerrainType exceeds internal terrain data. " << tt);
				return tiles_[tt];
			}
		private:
			point tile_size_;
			std::vector<terrain_tile_ptr> tiles_;
			// Map from name to terrain type for the tile.
			std::map<std::string, TerrainType> tile_name_map_;
			// List of terrain priorities. 
			// i.e. if we two tiles next to each other which one overlaps
			// the other.
			std::vector<TerrainType> tile_priority_list_;
			};

		terrain_data& get_terrain_data()
		{
			static terrain_data res;
			return res;
		}
	}

	terrain_tile::terrain_tile(float threshold, TerrainType tt, 
		const std::string& name, 
		const std::string& image, 
		const std::string& transitions_file)
		: threshold_(threshold), 
			terrain_type_(tt), 
			name_(name) 
		{
			surface_ = std::make_shared<graphics::surface>(image);
			if(!transitions_file.empty()) {
				transitions_ = std::make_shared<graphics::surface>(transitions_file);
			}
		}


	chunk::chunk(const point&pos, int width, int height)
		: pos_(pos),
		  width_(width),
		  height_(height),
		  texture_(nullptr)
	{
		terrain_.resize(height);
		for(auto& row : terrain_) {
			row.resize(width);
		}
	}

	chunk::~chunk()
	{
		if(texture_) {
			SDL_DestroyTexture(texture_);
			texture_ = nullptr;
		}
	}

	terrain_tile_ptr chunk::get_at(int x, int y)
	{
		ASSERT_LOG(x < width_, "x exceeds width of chunk: " << x << " >= " << width_);
		ASSERT_LOG(y < height_, "y exceeds height of chunk: " << y << " >= " << height_);
		return terrain_[y][x];
	}

	TerrainType chunk::get_terrain_at(int x, int y)
	{
		ASSERT_LOG(x < width_, "x exceeds width of chunk: " << x << " >= " << width_);
		ASSERT_LOG(y < height_, "y exceeds height of chunk: " << y << " >= " << height_);
		return terrain_[y][x]->get_terrain_type();
	}

	void chunk::set_at(int x, int y, float height_value)
	{
		ASSERT_LOG(x < width_, "x exceeds width of chunk: " << x << " >= " << width_);
		ASSERT_LOG(y < height_, "y exceeds height of chunk: " << y << " >= " << height_);
		terrain_[y][x] = get_terrain_data().get_tile_at_height(height_value);
	}

	surface_ptr chunk::make_surface_from_chunk(chunk_ptr chk)
	{
		auto& cache = get_terrain_data();
		auto& tile_size = cache.get_tile_size();
		int w = tile_size.x * chk->width();
		int h = tile_size.y * chk->height();

		surface_ptr dst = std::make_shared<graphics::surface>(w, h);
		for(int y = 0; y != chk->height(); ++y) {
			for(int x = 0; x != chk->width(); ++x) {
				dst->blit_scaled(chk->get_at(x, y)->get_surface(), 
					rect(x * tile_size.x, y * tile_size.y, tile_size.x, tile_size.y));
			}
		}
		return dst;
	}

	terrain_tile_ptr terrain::get_tile_at(const point& p)
	{
		int cw2 = chunk_size_w_/2;
		int ch2 = chunk_size_h_/2;
		// position of chunk
		point cpos(p.x > 0 ? (p.x+cw2)/cw2-1 : (p.x+cw2)/cw2, p.y > 0 ? (p.y+ch2)/ch2-1 : (p.y+ch2)/ch2);
		// position in chunk
		point pos(p.x % cw2 + cw2, p.y % ch2 + ch2);
		ASSERT_LOG(pos.x >= 0 && pos.x < chunk_size_w_, "x coordinate outside of chunk bounds. 0 <= " << pos.x << " < " << chunk_size_w_);
		ASSERT_LOG(pos.y >= 0 && pos.y < chunk_size_h_, "y coordinate outside of chunk bounds. 0 <= " << pos.y << " < " << chunk_size_h_);
		auto it = chunks_.find(cpos);
		if(it == chunks_.end()) {
			// is no an existing chunk, create a new one
			auto nchunk = generate_terrain_chunk(cpos);
			return nchunk->get_at(pos.x, pos.y);
		}
		return it->second->get_at(pos.x, pos.y);
	}

	namespace {
		std::vector<point>& get_edge_offsets()
		{
			static std::vector<point> res;
			if(res.empty()) {
				res.emplace_back(point(-1,0));	// west
				res.emplace_back(point(0,-1));	// north
				res.emplace_back(point(1,0));	// east
				res.emplace_back(point(0,1));	// south
			}
			return res;
		}
		std::vector<point>& get_corner_offsets()
		{
			static std::vector<point> res;
			if(res.empty()) {
				res.emplace_back(point(-1,-1));	// north-west
				res.emplace_back(point(1,-1));	// north-east
				res.emplace_back(point(1,1));	// south-east
				res.emplace_back(point(-1,1));	// south-west
			}
			return res;
		}

		const std::vector<point>& get_direction_offsets()
		{
			static std::vector<point> res;
			if(res.empty()) {
				res.emplace_back(point(-1,0));	// west
				res.emplace_back(point(0,-1));	// north
				res.emplace_back(point(1,0));	// east
				res.emplace_back(point(0,1));	// south
				res.emplace_back(point(-1,-1));	// north-west
				res.emplace_back(point(1,-1));	// north-east
				res.emplace_back(point(1,1));	// south-east
				res.emplace_back(point(-1,1));	// south-west
			}
			return res;
		}
	}

	surface_ptr terrain::build_transitions(chunk_ptr chk)
	{
		profile::manager pman("build_transitions_inner");
		auto& cache = get_terrain_data();
		auto& tile_size = cache.get_tile_size();
		int w = tile_size.x * chk->width();
		int h = tile_size.y * chk->height();
		surface_ptr dst = std::make_shared<graphics::surface>(w, h);
		for(int y = 0; y != chk->height(); ++y) {
			for(int x = 0; x != chk->width(); ++x) {
				const auto ctt = chk->get_at(x,y)->get_terrain_type();

				// XXX There feels like there should be a more elegant way
				// of doing this than constructing histograms.

				std::vector<int> histogram(get_terrain_data().get_priority_list().size(), 0);
				TerrainType tt = 0;
				for(auto& dir : get_direction_offsets()) {
					auto tp = get_tile_at(chk->get_position() + dir);
					if(tp->get_terrain_type() != ctt) {
						histogram[tp->get_terrain_type()] |= (1 << tt);
					}
					tt++;
				}

				tt = 0;
				for(auto h : histogram) {
					if(get_terrain_data().is_higher_priority(tt, ctt)) {
						// ch.second is now the index into the tile position on the surface
						const int ey = 0, ey2 = tile_size.y;
						const int cy = tile_size.y, cy2 = 2*tile_size.y;
						int eh = h & 0x7f;
						rect src_rect(tile_size.x*eh, ey, tile_size.x*(eh+1), ey2);
						rect dst_rect(x * tile_size.x, y * tile_size.y, tile_size.x, tile_size.y);
						auto ts = get_terrain_data().get_tile_from_terrain(tt)->get_transitions();
						if(ts) {
							dst->blit_scaled(ts, src_rect, dst_rect);
						}
						int ch = h >> 4;
						src_rect = rect(tile_size.x*ch, cy, tile_size.x*(ch+1), cy2);
						dst_rect = rect(x * tile_size.x, y * tile_size.y, tile_size.x, tile_size.y);
						ts = get_terrain_data().get_tile_from_terrain(tt)->get_transitions();
						if(ts) {
							dst->blit_scaled(ts, src_rect, dst_rect);
						}
					}
					++tt;
				}
			}
		}
		return dst;
	}

	void chunk::draw(const engine& eng, const point& cam) const
	{
		if(!texture_) {
			if(transitions_surface_) {
				surface_ptr nsurf = std::make_shared<graphics::surface>(chunk_surface_->width(), chunk_surface_->height());
				nsurf->blit_clipped(chunk_surface_);
				nsurf->blit_clipped(transitions_surface_);
				texture_ = SDL_CreateTextureFromSurface(const_cast<SDL_Renderer*>(eng.get_renderer()), nsurf->get());
			} else {
				texture_ = SDL_CreateTextureFromSurface(const_cast<SDL_Renderer*>(eng.get_renderer()), chunk_surface_->get());
			}			
		}
		ASSERT_LOG(texture_ != nullptr, "No texture found for chunk.");
		// Note camera isn't adjusted for tile size.
		// But it is in the same units as pos_ is;

		SDL_Rect dst = {
			(pos_.x - cam.x) * eng.get_tile_size().x - chunk_surface_->width() / 2 + eng.get_window().width() / 2 - eng.get_tile_size().x/2,
			(pos_.y - cam.y) * eng.get_tile_size().y - chunk_surface_->height() / 2 + eng.get_window().height() / 2 - eng.get_tile_size().y/2,
			chunk_surface_->width(),
			chunk_surface_->height()
		};
		SDL_RenderCopy(const_cast<SDL_Renderer*>(eng.get_renderer()), texture_, NULL, &dst);
	}

	std::size_t point_hash::operator()(const point& p) const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, boost::hash_value(p.x));
		boost::hash_combine(seed, boost::hash_value(p.y));
		return seed;
	}

	terrain::terrain()
		: chunk_size_w_(16),
		  chunk_size_h_(16)
	{
	}

	void terrain::load_terrain_data(const node& n)
	{
		get_terrain_data().load(n);
	}

	std::vector<chunk_ptr> terrain::get_chunks_in_area(const rect& r)
	{
		// We assume that chunks are placed on a grid. So that 0,0 is the co-ordinates of the 
		// center of the first chunk, the chunk one up/one right would be at chunk_size_w_,chunk_size_h_
	
		std::vector<chunk_ptr> res;
		
		// So first step, we create a new rectangle based on our grid that is normalised to cover the rectangle r.
		rect r_norm = rect::from_coordinates((r.x() - chunk_size_w_/2) / chunk_size_w_,
			(r.y() - chunk_size_h_/2) / chunk_size_h_,
			(r.x2() + chunk_size_w_/2) / chunk_size_w_,
			(r.y2() + chunk_size_h_/2) / chunk_size_h_);
		// Next iterate over the grid positions and get all the relevant chunks
		for(int y = r_norm.y(); y < r_norm.y2(); ++y) {
			for(int x = r_norm.x(); x < r_norm.x2(); ++x) {
				point pos(x * chunk_size_w_, y * chunk_size_h_);
				auto it = chunks_.find(pos);
				if(it == chunks_.end()) {
					auto nchunk = generate_terrain_chunk(pos);
					res.emplace_back(nchunk);
				} else {
					res.emplace_back(it->second);
				}				
			}
		}
		for(auto chk : res) {
			if(!chk->get_transitions_surface()) {
				auto surf = build_transitions(chk);
				chk->set_transitions_surface(surf);
			}
		}
		// XXX Need to add some chunk unloading logic.
		return res;
	}

	chunk_ptr terrain::generate_terrain_chunk(const point& pos)
	{
		using namespace noise;
		module::Perlin pnoise;
		pnoise.SetSeed(static_cast<int>(generator::get_seed()));
		chunk_ptr nchunk = std::make_shared<chunk>(pos, chunk_size_w_, chunk_size_h_);
		for(int y = 0; y < chunk_size_h_; ++y) {
			for(int x = 0; x < chunk_size_w_; ++x) {
				auto ns = pnoise.GetValue(
					(x + pos.x - chunk_size_w_ / 2.0) / (chunk_size_w_ * terrain_scale_factor),
					(y + pos.y - chunk_size_h_ / 2.0) / (chunk_size_h_ * terrain_scale_factor),
					0.0);
				nchunk->set_at(x, y, static_cast<float>(ns));
			}
		}
		nchunk->set_surface(chunk::make_surface_from_chunk(nchunk));
		chunks_[pos] = nchunk;
		return nchunk;
	}

	point terrain::get_terrain_size()
	{
		return get_terrain_data().get_tile_size();
	}
}
