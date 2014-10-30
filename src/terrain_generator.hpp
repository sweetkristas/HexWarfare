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
#include <unordered_map>

#include "color.hpp"
#include "node.hpp"
#include "geometry.hpp"
#include "surface.hpp"

class engine;

namespace terrain
{
	typedef unsigned TerrainType;

	class chunk;
	typedef std::shared_ptr<chunk> chunk_ptr;

	struct point_hash
	{
		std::size_t operator()(const point& p) const;
	};
	typedef std::unordered_map<point, chunk_ptr, point_hash> terrain_map_type;


	class terrain_tile
	{
	public:
		terrain_tile(float threshold, TerrainType tt, 
			const std::string& name, 
			const std::string& image, 
			const std::string& transitions_file);
		TerrainType get_terrain_type() const { return terrain_type_; }
		float get_threshold() const { return threshold_; }
		const std::string& get_name() const { return name_; }
		surface_ptr get_surface() { return surface_; }
		surface_ptr get_transitions() { return transitions_; }
	private:
		float threshold_;
		TerrainType terrain_type_;
		std::string name_;
		surface_ptr surface_;
		surface_ptr transitions_;
	};
	typedef std::shared_ptr<terrain_tile> terrain_tile_ptr;
	inline bool operator<(const terrain_tile& lhs, const terrain_tile& rhs) { return lhs.get_threshold() < rhs.get_threshold(); }
	inline bool operator<(const terrain_tile_ptr& lhs, const terrain_tile_ptr& rhs) { return *lhs < *rhs; }

	class chunk
	{
	public:
		chunk(const point& pos, int width, int height);
		~chunk();
		void set_at(int x, int y, float tt);
		terrain_tile_ptr get_at(int x, int y);
		TerrainType get_terrain_at(int x, int y);
		int width() const { return width_; }
		int height() const { return height_; }
		const point& get_position() const { return pos_; }
		void set_surface(surface_ptr surf) { chunk_surface_ = surf; }
		surface_ptr get_surface() { return chunk_surface_; }
		void set_transitions_surface(surface_ptr surf) { transitions_surface_ = surf; }
		surface_ptr get_transitions_surface() { return transitions_surface_; }
		static surface_ptr make_surface_from_chunk(chunk_ptr chk);
		void draw(const engine& eng, const point& cam) const;
	private:
		point pos_;
		int width_;
		int height_;
		std::vector<std::vector<terrain_tile_ptr>> terrain_;
		// should make this a texture.
		surface_ptr chunk_surface_;
		surface_ptr transitions_surface_;
		mutable SDL_Texture* texture_;
	};
	typedef std::shared_ptr<chunk> chunk_ptr;

	class terrain
	{
	public:
		terrain();
		//terrain(const node& n);

		// Find all the chunks which are in the given area, including partials.
		// Will generate chunks as needed for complete coverage.
		std::vector<chunk_ptr> get_chunks_in_area(const rect& r);

		// Pos is the worldspace position.
		chunk_ptr generate_terrain_chunk(const point& pos);
		terrain_tile_ptr get_tile_at(const point& p);
		surface_ptr build_transitions(chunk_ptr chk);

		static void load_terrain_data(const node& n);
		static point get_terrain_size();
		//node write();
	private:
		int chunk_size_w_;
		int chunk_size_h_;
		terrain_map_type chunks_;
	};
}