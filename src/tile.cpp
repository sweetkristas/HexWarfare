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

#include <vector>

namespace tile
{
	namespace 
	{
		// Maps from the 256 possible combinations down to the "standard" 47 tile combinations.
#if defined(_MSC_VER) && _MSC_VER <= 1700
		// No initialiser lists in VS2012 or lower.
		const int tile_mapping[256] = {
#else
		const std::vector<int> tile_mapping = {
#endif
			 0,  1,  2,  2,  3,  4,  2,  2,  5,  5,  6,  6,  7,  7,  6,  6, 
			 8,  9, 10, 10,  8,  9, 10, 10, 11, 11, 12, 12, 11, 11, 12, 12, 
			13, 14, 15, 15, 16, 17, 15, 15,  5,  5,  6,  6,  7,  7,  6,  6, 
			18, 19, 20, 20, 18, 19, 20, 20, 11, 11, 12, 12, 11, 11, 12, 12, 
			21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27, 
			29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33, 
			21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27, 
			29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33, 
			34, 35, 36, 36, 37, 38, 36, 36, 39, 39, 40, 40, 41, 41, 40, 40,
			 8,  9, 10, 10,  8,  9, 10, 10, 11, 11, 12, 12, 11, 11, 12, 12, 
			42, 43, 44, 44, 45, 46, 44, 44, 39, 39, 40, 40, 41, 41, 40, 40, 
			18, 19, 20, 20, 18, 19, 20, 20, 11, 11, 12, 12, 11, 11, 12, 12, 
			21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27, 
			29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33, 
			21, 22, 23, 23, 24, 25, 23, 23, 26, 26, 27, 27, 28, 28, 27, 27, 
			29, 30, 31, 31, 29, 30, 31, 31, 32, 32, 33, 33, 32, 32, 33, 33, 
		};

	}

	// 32 tile solution.
	// For edges	bit 0 = west, bit 1 = north, bit 2 = east, bit 3 = south
	// For corners	bit 0 = north-west, bit 1 = north-east, bit 2 = south-east, bit 3 = south-west
}
