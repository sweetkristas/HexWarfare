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
#include <set>
#include <vector>

#include "geometry.hpp"
#include "node.hpp"
#include "texture.hpp"

namespace castle
{
	class keep;
	class castle;
	typedef std::shared_ptr<castle> castle_ptr;

	class tile
	{
	public:
		tile();
		explicit tile(const graphics::texture& t);
		const graphics::texture& texture() const { return tex_; }
	private:
		graphics::texture tex_;
	};

	class castle
	{
	public:
		explicit castle(const node& value);

		static castle_ptr factory(const node& value);

		void draw(const point& p) const;

		node write() const;
	private:
		std::vector<std::pair<point, tile>> tiles_;
		std::set<point> base_positions_;
	};

	void loader(SDL_Renderer* renderer, const node& n);
}
