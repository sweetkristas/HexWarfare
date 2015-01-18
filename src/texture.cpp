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

#include <map>

#include "SDL_image.h"

#include "asserts.hpp"
#include "color.hpp"
#include "profile_timer.hpp"
#include "texture.hpp"

namespace graphics
{
	namespace
	{
		typedef std::map<std::string, std::shared_ptr<SDL_Texture>> texture_cache_type;

		texture_cache_type& texture_cache()
		{
			static texture_cache_type res;
			return res;
		}
		
		SDL_Renderer*& get_renderer()
		{
			static SDL_Renderer* res = nullptr;
			return res;
		}
	}

	texture::manager::manager(SDL_Renderer* renderer)
	{
		get_renderer() = renderer;
	}

	texture::manager::~manager()
	{
	}

	texture::texture()
	{
	}

	void texture::load_file_into_texture(const std::string& fname, texture* tex)
	{
		SDL_Surface* source = IMG_Load(fname.c_str());
		ASSERT_LOG(source != NULL, "Failed to load image: " << fname << " : " << IMG_GetError());
		texture_from_surface(source, tex);
	}

	void texture::texture_from_surface(SDL_Surface* source, texture* tex)
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_BLEND);
		//SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_NONE);
		// If the source area is empty then default to the whole image.
		if(tex->area_.empty()) {
			tex->area_ = rect(0, 0, source->w, source->h);
		}
		
		auto ntex = SDL_CreateTextureFromSurface(get_renderer(), source);
		ASSERT_LOG(ntex != nullptr, "Couldn't create texture: " << SDL_GetError());
		tex->tex_.reset(ntex, [](SDL_Texture* t) {
			SDL_DestroyTexture(t);
		});
	}

	texture::texture(const std::string& fname, TextureFlags flags, const rect& area)
		: area_(area), 
		  flags_(flags),
		  blend_mode_(SDL_BLENDMODE_BLEND)
	{
		if(flags & TextureFlags::NO_CACHE) {
			load_file_into_texture(fname, this);
			return;
		}
		auto it = texture_cache().find(fname);
		if(it == texture_cache().end()) {
			load_file_into_texture(fname, this);
			texture_cache()[fname] = tex_;
		} else {
			// XXX we should cache the area the texture occupies.
			tex_ = it->second;
			int w = 0;
			int h = 0;
			if(!SDL_QueryTexture(tex_.get(), NULL, NULL, &w, &h)) {
				area_.set_wh(w, h);
			} else {
				ASSERT_LOG(false, "SDL error querying texture: " << SDL_GetError());
			}
		}
	}

	texture::texture(int w, int h, TextureFlags flags)
		: flags_(flags),
		  area_(rect(0, 0, w, h)),
		  blend_mode_(SDL_BLENDMODE_BLEND)
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		auto ntex = SDL_CreateTexture(get_renderer(), 
			SDL_PIXELFORMAT_ARGB8888, 
			(flags_ & TextureFlags::TARGET) ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STATIC, 
			w, 
			h);
		ASSERT_LOG(ntex != nullptr, "Couldn't create texture: " << SDL_GetError());
		tex_.reset(ntex, [](SDL_Texture* t) {
			SDL_DestroyTexture(t);
		});
	}

	texture::texture(const surface_ptr& surf, TextureFlags flags, const rect& area)
		: area_(area), 
		  flags_(flags),
		  blend_mode_(SDL_BLENDMODE_BLEND)
	{
		texture_from_surface(const_cast<SDL_Surface*>(surf->get()), this);
	}

	void texture::rebuild_cache()
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		for(auto it = texture_cache().begin(); it != texture_cache().end(); ++it) {
			SDL_Surface* source = IMG_Load(it->first.c_str());
			ASSERT_LOG(source != NULL, "Failed to load image: " << it->first << " : " << IMG_GetError());
			SDL_SetSurfaceBlendMode(source, SDL_BLENDMODE_BLEND);
			it->second.reset(SDL_CreateTextureFromSurface(get_renderer(), source), [](SDL_Texture* t) {
				SDL_DestroyTexture(t);
			});
		}
	}

	void texture::update(const std::string& fname, const rect& area)
	{
		set_area(area);
		load_file_into_texture(fname, this);
		if(!(flags_ & TextureFlags::NO_CACHE)) {
			texture_cache()[fname] = tex_;
		}
	}

	void texture::update(const surface_ptr& surf, const rect& area)
	{
		set_area(area);
		texture_from_surface(const_cast<SDL_Surface*>(surf->get()), this);
	}

	void texture::set_alpha(int alpha)
	{
		alpha = std::min(std::max(0, alpha), 255);
		SDL_SetTextureAlphaMod(tex_.get(), static_cast<Uint8>(alpha));
	}

	void texture::set_color(const color& col)
	{
		SDL_SetTextureColorMod(tex_.get(), col.r(), col.g(), col.b());
	}

	void texture::blit(const rect& dest) const
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		SDL_Rect src = {area_.x(), area_.y(), area_.w(), area_.h()};
		SDL_Rect dst = {dest.x(), dest.y(), dest.w() == 0 ? area_.w() : dest.w(), dest.h() == 0 ? area_.h() : dest.h()};
		int res = SDL_SetTextureBlendMode(tex_.get(), blend_mode_);
		ASSERT_LOG(res == 0, "Blend mode couldn't be set: " << SDL_GetError());
		res = SDL_RenderCopy(get_renderer(), tex_.get(), &src, &dst);
		ASSERT_LOG(res == 0, "Failed to blit texture: " << SDL_GetError());
	}

	void texture::blit(const rect& src_r, const rect& dest_r) const
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		SDL_Rect src = {src_r.x(), src_r.y(), src_r.w(), src_r.h()};
		SDL_Rect dst = {dest_r.x(), dest_r.y(), dest_r.w() == 0 ? src_r.w() : dest_r.w(), dest_r.h() == 0 ? src_r.h() : dest_r.h()};
		int res = SDL_SetTextureBlendMode(tex_.get(), blend_mode_);
		ASSERT_LOG(res == 0, "Blend mode couldn't be set: " << SDL_GetError());
		res = SDL_RenderCopy(get_renderer(), tex_.get(), &src, &dst);
		ASSERT_LOG(res == 0, "Failed to blit texture: " << SDL_GetError());
	}

	void texture::blit_ex(const rect& dest, double angle, const point& center, FlipFlags flip) const
	{
		ASSERT_LOG(get_renderer() != nullptr, "Renderer not set. call graphics::texture::manager texman(...);");
		SDL_Rect src = {area_.x(), area_.y(), area_.w(), area_.h()};
		SDL_Rect dst = {dest.x(), dest.y(), dest.w() == 0 ? area_.w() : dest.w(), dest.h() == 0 ? area_.h() : dest.h()};
		SDL_Point pt = {center.x, center.y};
		SDL_RendererFlip ff = static_cast<SDL_RendererFlip>((flip & FlipFlags::HORIZONTAL ? SDL_FLIP_HORIZONTAL : 0) 
			| (flip & FlipFlags::VERTICAL ? SDL_FLIP_VERTICAL : 0));
		int res = SDL_SetTextureBlendMode(tex_.get(), blend_mode_);
		ASSERT_LOG(res == 0, "Blend mode couldn't be set: " << SDL_GetError());
		res = SDL_RenderCopyEx(get_renderer(), tex_.get(), &src, &dst, angle, &pt, ff);
		ASSERT_LOG(res == 0, "Failed to blit texture: " << SDL_GetError());
	}

	void texture::set_blend(BlendMode bm)
	{
		SDL_BlendMode sbm = SDL_BLENDMODE_NONE;
		switch(bm) {
			case BlendMode::NONE:		sbm = SDL_BLENDMODE_NONE; break;
			case BlendMode::BLEND:		sbm = SDL_BLENDMODE_BLEND; break;
			case BlendMode::ADDITIVE:	sbm = SDL_BLENDMODE_ADD; break;
			case BlendMode::MODULATE:	sbm = SDL_BLENDMODE_MOD; break;
			default: ASSERT_LOG(false, "Unrecognised constant for blend mode: " << static_cast<int>(bm));
		}
		blend_mode_ = sbm;
	}

	void texture::set_area(const rect& area)
	{
		area_ = area;
	}
}
