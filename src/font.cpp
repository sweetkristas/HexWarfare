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

#include "asserts.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "notify.hpp"

namespace font
{
	namespace
	{
		const char* default_font_path = "data/fonts/";

		typedef std::pair<std::string, int> font_pair;
		typedef std::map<font_pair, font_ptr> font_map;
		font_map font_table;

		std::map<std::string,std::string>& get_font_list()
		{
			static std::map<std::string,std::string> res;
			return res;
		}

		void font_file_changed(const std::string& file, const boost::asio::dir_monitor_event& ev)
		{
			if(ev.type == boost::asio::dir_monitor_event::modified) {
				// XXX
				auto& res = get_font_list();
				sys::get_unique_files(default_font_path, res);
			}
		}

		const std::string& get_font_path(const std::string& name) 
		{
			auto& res = get_font_list();
			if(res.empty()) {
				sys::get_unique_files(default_font_path, res);
				notify::register_notification_path(default_font_path, font_file_changed);
			}
			auto itor = res.find(name);
			if(itor == res.end()) {
				ASSERT_LOG(false, "Font file not found: " << name);
			}
			return itor->second;
		}
	}

	font_ptr get_font(const std::string& font_name, int size)
	{
		const std::string& font_path = get_font_path(font_name);
		auto it = font_table.find(std::make_pair(font_path, size));
		if(it == font_table.end()) {
			TTF_Font* font = TTF_OpenFont(font_path.c_str(), size);
			ASSERT_LOG(font != NULL, "Unable to open font: " << font_name);
			font_ptr new_font(font, TTF_CloseFont);
			font_table[std::make_pair(font_path, size)] = new_font;
			return new_font;
		}
		return it->second;
	}

	SDL_Surface* render(const std::string& utf8, const font_ptr& fnt, const graphics::color& color)
	{
		return TTF_RenderUTF8_Blended(fnt.get(), utf8.c_str(), color.as_sdl_color());
	}

	SDL_Surface* render_shaded(const std::string& utf8, const font_ptr& fnt, const graphics::color& fg, const graphics::color& bg)
	{
		return TTF_RenderUTF8_Shaded(fnt.get(), utf8.c_str(), fg.as_sdl_color(), bg.as_sdl_color());
	}

	void get_glyph_metrics(const font_ptr& fnt, int ch, int *w, int *h, glyph_metrics* gm)
	{
		// Excludes kerning for obvious(1) reasons.
		// (1) obvious in this context means that we're only considering one character and kerning
		//     requires knowledge of a pair of characters.
		int minx, maxx, miny, maxy, advance;
		auto res = TTF_GlyphMetrics(fnt.get(), ch, &minx, &maxx, &miny, &maxy, &advance);
		ASSERT_LOG(res == 0, "Error getting font metrics for character '" << static_cast<char>(ch) << "' : " << SDL_GetError());
		if(w) {
			*w = advance;
		}
		if(h) {
			*h = TTF_FontHeight(fnt.get());
		}
		if(gm) {
			gm->minx = minx;
			gm->maxx = maxx;
			gm->miny = maxy;
			gm->minx = minx;
			gm->advance = advance;
		}
	}

	void get_text_size(const font_ptr& fnt, const std::string& utf8, int* w, int* h)
	{
		auto res = TTF_SizeText(fnt.get(), utf8.c_str(), w, h);
		ASSERT_LOG(res == 0, "Unable to get text size of string: '" << utf8 << "' : " << SDL_GetError());
	}

	std::string get_default_font_name()
	{
		// XXX load this from a config file.
		return "SourceCodePro-Regular.ttf";
	}

	manager::manager()
	{
		ASSERT_LOG(TTF_Init() != -1, "TTF_Init error(): " << TTF_GetError());
	}

	manager::~manager()
	{
		font_table.clear();
		TTF_Quit();
	}
}
