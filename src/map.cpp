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
#include "map.hpp"

// XXX change these to a boost:bimap

void init_map_symbols(std::map<char, MapSymbols>& res)
{
	res['.'] = MapSymbols::DIRT;
	res['#'] = MapSymbols::STONE;
	res['~'] = MapSymbols::WATER;
	res['_'] = MapSymbols::GRASS;

	res['0'] = MapSymbols::EXIT;
	res['1'] = MapSymbols::START;
}

MapSymbols convert_map_symbol(char c)
{
	static std::map<char, MapSymbols> res;
	if(res.empty()) {
		init_map_symbols(res);
	}
	auto it = res.find(c);
	ASSERT_LOG(it != res.end(), "Unable to map symbol '" << c << "' to a valid constant.");
	return it->second;
}

void init_map_chars(std::map<MapSymbols, char>& res)
{
	res[MapSymbols::DIRT]  = '.';
	res[MapSymbols::STONE] = '#';
	res[MapSymbols::WATER] = '~';
	res[MapSymbols::GRASS] = '_';

	res[MapSymbols::EXIT]  = '0';
	res[MapSymbols::START] = '1';
}

char convert_map_symbol_to_char(MapSymbols m)
{
	static std::map<MapSymbols, char> res;
	if(res.empty()) {
		init_map_chars(res);
	}
	auto it = res.find(m);
	ASSERT_LOG(it != res.end(), "Unable to map symbol from '" << static_cast<int>(m) << "' to a valid constant.");
	return it->second;
}

bool is_passable(MapSymbols m)
{
	if(m == MapSymbols::DIRT || m == MapSymbols::WATER || m == MapSymbols::GRASS) { 
		return true;
	}
	return false;
}
