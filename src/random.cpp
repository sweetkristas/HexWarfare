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

#include "asserts.hpp"
#include "random.hpp"

namespace generator
{
	namespace
	{
		bool seed_set = false;
		std::size_t seed_internal = 0;
	}

	std::mt19937& get_random_engine()
	{
		static std::mt19937 random_engine(seed_internal);
		ASSERT_LOG(seed_set, "No seed set");
		return random_engine;
	}

	std::size_t get_seed()
	{
		return seed_internal;
	}

	void set_seed(std::size_t seed)
	{
		seed_internal = seed;
		seed_set = true;
	}

	std::size_t generate_seed()
	{
		seed_internal = std::default_random_engine()();
		seed_set = true;
		return seed_internal;
	}
}
