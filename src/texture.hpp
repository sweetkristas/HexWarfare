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

#include "geometry.hpp"
#include "surface.hpp"

namespace graphics
{
	enum class TextureFlags
	{
		NONE		= 0,
		NO_CACHE	= 1,
		TARGET		= 2,
	};
	
	inline TextureFlags operator|(TextureFlags lhs, TextureFlags rhs)
	{
		return static_cast<TextureFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	inline bool operator&(TextureFlags lhs, TextureFlags rhs)
	{
		return ((static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs));
	}

	enum class FlipFlags
	{
		NONE		= 0,
		HORIZONTAL	= 1,
		VERTICAL	= 2,
	};

	inline FlipFlags operator|(FlipFlags lhs, FlipFlags rhs)
	{
		return static_cast<FlipFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
	}

	inline bool operator&(FlipFlags lhs, FlipFlags rhs)
	{
		return ((static_cast<int>(lhs) & static_cast<int>(rhs)) == static_cast<int>(rhs));
	}

	enum class BlendMode
	{
		NONE,			// no blending
		BLEND,			// alpha blending
		ADDITIVE,		// additive blending
		MODULATE,		// color modulation
	};

	class texture
	{
	public:
		struct manager
		{
			manager(SDL_Renderer* r);
			~manager();
		};

		texture();
		explicit texture(int w, int h, TextureFlags flags);
		explicit texture(const std::string& fname, TextureFlags flags, const rect& area=rect());
		explicit texture(const surface_ptr& surf, TextureFlags flags, const rect& area=rect());

		const rect& get_area() const { return area_; }
		void set_area(const rect& area);

		void update(const std::string& fname, const rect& area = rect());
		void update(const surface_ptr& surf, const rect& area = rect());

		void set_blend(BlendMode bm);

		void set_alpha(int alpha);

		SDL_Texture* get() { return tex_.get(); }

		bool is_valid() const { return tex_ != nullptr; }

		void set_name(const std::string& name) { name_ = name; }
		const std::string& get_name() const { return name_; }

		int width() const { return area_.w(); }
		int height() const { return area_.h(); }

		void blit(const rect& dst) const;
		void blit(const rect& src_r, const rect& dest_r) const;
		void blit_ex(const rect& dst, double angle, const point& center, FlipFlags flip) const;
		static void rebuild_cache();
	private:
		static void texture_from_surface(SDL_Surface* source, texture* tex);
		static void load_file_into_texture(const std::string& fname, texture* tex);
		rect area_;
		TextureFlags flags_;
		std::shared_ptr<SDL_Texture> tex_;
		std::string name_;
		SDL_BlendMode blend_mode_;
	};
}
