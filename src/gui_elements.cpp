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

#include <algorithm>
#include <map>

#include "asserts.hpp"
#include "gui_elements.hpp"
#include "texpack.hpp"

namespace gui
{
	namespace section
	{
		namespace 
		{
			typedef std::map<std::string, graphics::texture> section_map;
			section_map& get_section_map()
			{
				static section_map res;
				return res;
			}
		}

		manager::manager(SDL_Renderer* renderer, const node& n)
		{
			SDL_RendererInfo info;
			int res = SDL_GetRendererInfo(renderer, &info);
			ASSERT_LOG(res == 0, "Failed to get renderer info: " << SDL_GetError());

			ASSERT_LOG(n.has_key("sections") && n["sections"].is_list(), 
				"Must be 'sections' attribute in gui file which is a list.");
			std::vector<std::pair<std::string, surface_ptr>> surfs;
			for(auto& s : n["sections"].as_list()) {
				rect area = s.has_key("area") ? rect(s["area"].as_list_ints()) : rect();
				surfs.emplace_back(std::make_pair(s["name"].as_string(), std::make_shared<graphics::surface>(s["image"].as_string(), area)));
			}
			for(auto& vtex : graphics::packer<std::string>(surfs, info.max_texture_width, info.max_texture_height)) {
				for(auto& tex : vtex) {
					get_section_map()[tex.first] = tex.second;
				}
			}
		}

		manager::~manager()
		{
		}

		graphics::texture get(const std::string& name)
		{
			auto it = get_section_map().find(name);
			ASSERT_LOG(it != get_section_map().end(), "Unable to find a gui texture with name: " << name);
			return it->second;
		}
	}
}
