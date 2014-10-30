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
#include "SDL_ttf.h"

#include "color.hpp"
#include "surface.hpp"

namespace font
{
	class manager
	{
	public:
		manager();
		~manager();
	};

	struct glyph_metrics 
	{
		int minx;
		int maxx;
		int miny;
		int maxy;
		int advance;
	};

	typedef std::shared_ptr<TTF_Font> font_ptr;
	std::string get_default_font_name();
	font_ptr get_font(const std::string& font_name, int size);
	void get_glyph_metrics(const font_ptr& fnt, int ch, int *w, int *h, glyph_metrics* gm=NULL);
	void get_text_size(const font_ptr& fnt, const std::string& utf8, int* w, int* h);
	SDL_Surface* render(const std::string& utf8, const font_ptr& fnt, const graphics::color& color);
	SDL_Surface* render_shaded(const std::string& utf8, const font_ptr& fnt, const graphics::color& fg, const graphics::color& bg);
}
