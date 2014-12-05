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
#include "SDL.h"

#include "geometry.hpp"

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#	define SURFACE_MASK 0xFF,0xFF00,0xFF0000,0xFF000000
#	define SURFACE_MASK_RGB 0xFF,0xFF00,0xFF0000,0x0
#	define SURFACE_MASK_R 0xff
#	define SURFACE_MASK_G 0xff00
#	define SURFACE_MASK_B 0xff0000
#	define SURFACE_MASK_A 0xff000000
#else
#	define SURFACE_MASK 0xFF000000,0xFF0000,0xFF00,0xFF
#	define SURFACE_MASK_RGB 0xFF0000,0xFF00,0xFF,0x0
#	define SURFACE_MASK_R 0xff000000
#	define SURFACE_MASK_G 0xff0000
#	define SURFACE_MASK_B 0xff00
#	define SURFACE_MASK_A 0xff
#endif

namespace graphics
{
	class surface;
}
typedef std::shared_ptr<graphics::surface> surface_ptr;

namespace graphics
{
	class surface
	{
	public:
		explicit surface(int width, int height);
		explicit surface(const std::string& fname, const rect& r=rect());
		explicit surface(SDL_Surface* surf);
		~surface();

		SDL_Surface* get() { return surf_.get(); }
		const SDL_Surface* get() const { return surf_.get(); }

		int width() const { return surf_->w; }
		int height() const { return surf_->h; }

		// clipped blit
		void blit_clipped(const surface_ptr& src, const rect& dst_rect=rect());
		// scaled blit
		void blit_scaled(const surface_ptr& src, const rect& dst_rect);
		void blit_scaled(const surface_ptr& src, const rect& src_rect, const rect& dst_rect);

		void save(const std::string& filename);

		static surface_ptr create(int width, int height);
		static surface_ptr create(const std::string& fname);
		static surface_ptr create(SDL_Surface* surf);
	private:
		surface();
		std::shared_ptr<SDL_Surface> surf_;
	};
}
