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

#include <string>
#include <vector>

enum class MapSymbols
{
	DIRT,
	STONE,
	WATER,
	GRASS,

	EXIT,
	START,

	TREE, // Still on the fence about whether this should be here or not

};

typedef std::vector<std::vector<MapSymbols>> map_type;

MapSymbols convert_map_symbol(char c);
char convert_map_symbol_to_char(MapSymbols sym);
bool is_passable(MapSymbols sym);
