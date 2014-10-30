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

namespace graphics
{
	class window_manager
	{
	public:
		window_manager();
		void create_window(const std::string& title, int x, int y, int w, int h, Uint32 flags);
		void gl_init();
		void set_icon(const std::string& icon);
		SDL_Window* get_window() { return window_; }
		SDL_Renderer* get_renderer() { return renderer_; }
		void swap();
		virtual ~window_manager();	
		int width() const { return width_; }
		int height() const { return height_; }
		const rect& get_viewport() const { return area_; }
		const point get_dim() const { return area_.dimensions(); }
		void update_window_size();
		static window_manager& get_main_window();
	private:
		SDL_Window* window_;
		SDL_GLContext glcontext_;
		SDL_Renderer* renderer_;
		
		// area_ is a synonym for (0,0,width_,height_)
		// we maintain it to give quick access to the screen
		// area rather than creating a temporary rect everytime.
		int width_;
		int height_;
		rect area_;
	};
}
