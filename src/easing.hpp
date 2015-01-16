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

#define _USE_MATH_DEFINES
#include <cmath>

namespace easing
{
	// t is the time from 0 to d.
	// d is the duration.
	// b is the starting value.
	// c is the change in value

	template<typename T, typename D=double>
	T linear_tween(D t, T b, T c, D d)
	{
		return c * (t / d) + b;
	}

	template<typename T, typename D=double>
	T ease_in_quad(D t, T b, T c, D d) 
	{
		t /= d;
		return c * t * t + b;
	}

	template<typename T, typename D=double>
	T ease_out_quad(D t, T b, T c, D d) 
	{
		t /= d;
		return -c * t * (t-D(2)) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_quad(D t, T b, T c, D d) 
	{
		t /= d / D(2);
		if(t < D(1)) {
			return c / D(2) * t * t + b;
		}
		return -c / D(2) * (t * (t-D(2)) - D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_cubic(D t, T b, T c, D d)
	{
		t /= d;
		return c * t * t * t + b;
	}

	template<typename T, typename D=double>
	T ease_out_cubic(D t, T b, T c, D d)
	{
		t /= d;
		t--;
		return c * (t * t * t + D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_cubic(D t, T b, T c, D d)
	{
		t /= d / D(2);
		if(t < D(1)) {
			return c / D(2) * t * t * t + b;
		}
		t -= D(2);
		return c / D(2) * (t * t * t + D(2)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_quartic(D t, T b, T c, D d)
	{
		t /= d;
		return c * t * t * t * t + b;
	}

	template<typename T, typename D=double>
	T ease_out_quartic(D t, T b, T c, D d)
	{
		t /= d;
		t--;
		return -c * (t * t * t * t - D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_quartic(D t, T b, T c, D d)
	{
		t /= d / D(2);
		if(t < D(1)) {
			return c/D(2) * t * t * t * t + b;
		}
		t -= D(2);
		return -c * (t * t * t * t - D(2)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_quintic(D t, T b, T c, D d)
	{
		t /= d;
		return c * t * t * t * t * t + b;
	}

	template<typename T, typename D=double>
	T ease_out_quintic(D t, T b, T c, D d)
	{
		t /= d;
		t--;
		return c * (t * t * t * t * t + D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_quintic(D t, T b, T c, D d)
	{
		t /= d / D(2);
		if(t < D(1)) {
			return c / D(2) * t * t * t * t * t + b;
		}
		t -= D(2);
		return c / D(2) * (t * t * t * t * t + D(2)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_sinusoidal(D t, T b, T c, D d)
	{
		return -c * std::cos(t/d * (D(M_PI / 2.0))) + c + b;
	}

	template<typename T, typename D=double>
	T ease_out_sinusoidal(D t, T b, T c, D d)
	{
		return c * std::sin(t / d * (D(M_PI / 2.0))) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_sinusoidal(D t, T b, T c, D d)
	{
		return -c / D(2) * (std::cos(t / d * D(M_PI)) - D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_exponential(D t, T b, T c, D d)
	{
		return c * std::pow(D(2), D(10) * (t / d - D(1))) + b;
	}

	template<typename T, typename D=double>
	T ease_out_exponential(D t, T b, T c, D d)
	{
		return c * (-std::pow(D(2), D(-10) * t / d) + D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_exponential(D t, T b, T c, D d)
	{
		t /= d / D(2);
		if(t < D(1)) {
			return c / D(2) * std::pow(D(2), D(10) * (t - D(1))) + b;
		}
		t -= D(1);
		return c / D(2) * (D(2) - std::pow(D(2), D(10) * t)) + b;
	}

	template<typename T, typename D=double>
	T ease_in_circular(D t, T b, T c, D d)
	{
		t /= d;
		return -c * (std::sqrt(D(1) - t * t) - D(1)) + b;
	}

	template<typename T, typename D=double>
	T ease_out_circular(D t, T b, T c, D d)
	{
		t /= d;
		t -= D(1);
		return c * std::sqrt(D(1) - t * t) + b;
	}

	template<typename T, typename D=double>
	T ease_inout_circular(D t, T b, T c, D d)
	{
		t /= d / D(2);
		if(t < D(1)) {
			return -c / D(2) * (std::sqrt(D(1) - t * t) - D(1)) + b;
		}
		t -= D(2);
		return c / D(2) * (std::sqrt(D(1) - t * t) + D(1)) + b;
	}

	/*
	template<typename T, typename D=double>
	T ease_(D t, T b, T c, D d)
	{
	}
	*/
}
