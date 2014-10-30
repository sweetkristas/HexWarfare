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

#include "SDL.h"

namespace graphics
{
	class color
	{
	public:
		color()
		{
			c_.rgba[0] = 255;
			c_.rgba[1] = 255;
			c_.rgba[2] = 255;
			c_.rgba[3] = 255;
			set_float_values();
		}
		explicit color(int r, int g, int b, int a=255)
		{
			c_.rgba[0] = truncate_to_char(r);
			c_.rgba[1] = truncate_to_char(g);
			c_.rgba[2] = truncate_to_char(b);
			c_.rgba[3] = truncate_to_char(a);
			set_float_values();
		}
		explicit color(uint32_t rgba)
		{
			c_.value = rgba;
			set_float_values();
		}
		explicit color(const SDL_Color& col)
		{
			c_.rgba[0] = col.r;
			c_.rgba[1] = col.g;
			c_.rgba[2] = col.b;
			c_.rgba[3] = 255;
			set_float_values();
		}
		explicit color(float r, float g, float b, float a = 1.0f)
		{
			rgbaf_[0] = clampf(r);
			rgbaf_[1] = clampf(g);
			rgbaf_[2] = clampf(b);
			rgbaf_[3] = clampf(a);
			c_.rgba[0] = int(rgbaf_[0] * 255.0f);
			c_.rgba[1] = int(rgbaf_[1] * 255.0f);
			c_.rgba[2] = int(rgbaf_[2] * 255.0f);
			c_.rgba[3] = int(rgbaf_[3] * 255.0f);
		}

		uint8_t r() const { return c_.rgba[0]; }
		uint8_t g() const { return c_.rgba[1]; }
		uint8_t b() const { return c_.rgba[2]; }
		uint8_t a() const { return c_.rgba[3]; }

		SDL_Color as_sdl_color() const
		{
			SDL_Color c = {c_.rgba[0], c_.rgba[1], c_.rgba[2], 255};
			return c;
		}

		const float* as_gl_color() const
		{
			return rgbaf_;
		}
		float* as_gl_color()
		{
			return rgbaf_;
		}

		uint8_t truncate_to_char(int value)
		{
			if(value < 0) return 0;
			if(value > 255) return 255;
			return value;
		}

	private:
		void set_float_values()
		{
			rgbaf_[0] = c_.rgba[0] / 255.0f;
			rgbaf_[1] = c_.rgba[1] / 255.0f;
			rgbaf_[2] = c_.rgba[2] / 255.0f;
			rgbaf_[3] = c_.rgba[3] / 255.0f;
		}

		float clampf(float value)
		{
			if(value < 0.0f) return 0.0f;
			if(value > 1.0f) return 1.0f;
			return value;
		}

		union PixelUnion
		{
			uint32_t value;
			uint8_t rgba[4];
		};
		float rgbaf_[4];

		PixelUnion c_;
	};
}
