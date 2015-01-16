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

#include "easing.hpp"
#include "geometry.hpp"

namespace easing
{
	namespace between
	{
		template<typename D=double>
		inline point linear_tween(D t, const point& p1, const point& p2, D d)
		{
			point c = p2 - p1;
			float x = static_cast<float>(t / d) * static_cast<float>(c.x) + static_cast<float>(p1.x);
			float y = static_cast<float>(t / d) * static_cast<float>(c.y) + static_cast<float>(p1.y);
			return point(static_cast<int>(x), static_cast<int>(y));
		}

		template<typename D=double>
		inline point ease_in_quad(D t, const point& p1, const point& p2, D d) 
		{
			point c = p2 - p1;
			t /= d;
			t *= t;
			float x = static_cast<float>(t) * static_cast<float>(c.x) + static_cast<float>(p1.x);
			float y = static_cast<float>(t) * static_cast<float>(c.y) + static_cast<float>(p1.y);
			return point(static_cast<int>(x), static_cast<int>(y));
		}

		template<typename D=double>
		inline point ease_out_quad(D t, const point& p1, const point& p2, D d) 
		{
			point c = p2 - p1;
			t /= d;
			t *= (t - D(2));
			float x = static_cast<float>(t) * -static_cast<float>(c.x) + static_cast<float>(p1.x);
			float y = static_cast<float>(t) * -static_cast<float>(c.y) + static_cast<float>(p1.y);
			return point(static_cast<int>(x), static_cast<int>(y));
		}
	}
}
