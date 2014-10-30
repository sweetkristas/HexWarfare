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

#include <random>

namespace generator
{
	std::size_t get_seed();
	void set_seed(std::size_t seed);
	std::size_t generate_seed();

	std::mt19937& get_random_engine();

	template<typename T>
	T get_uniform_int(T mn, T mx)
	{
		auto& re = get_random_engine();
		std::uniform_int_distribution<T> uniform_dist(mn, mx);
		return uniform_dist(re);
	}

	template<typename T>
	T get_uniform_real(T mn, T mx)
	{
		auto& re = get_random_engine();
		std::uniform_real_distribution<T> uniform_dist(mn, mx);
		return uniform_dist(re);
	}
}
