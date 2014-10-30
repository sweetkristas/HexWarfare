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

#include <memory>

namespace hex 
{
	enum direction { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, NORTH_WEST };

	class hex_map;
	class hex_object;
	class tile_sheet;
	class tile_type;

	typedef std::shared_ptr<hex_object> hex_object_ptr;
	typedef std::shared_ptr<hex_map> hex_map_ptr;
	typedef std::shared_ptr<const tile_sheet> tile_sheet_ptr;
	typedef std::shared_ptr<tile_type> tile_type_ptr;
}
