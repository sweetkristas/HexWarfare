/*
   Copyright 2009-2014 David White <davewx7@gmail.com>

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
#include <set>

#include "asserts.hpp"
#include "utility.hpp"

namespace utility
{
	namespace
	{
		typedef std::map<std::string, UtilityProgram> UtilityMap;
		UtilityMap& get_utility_map()
		{
			static UtilityMap map;
			return map;
		}

		std::set<std::string>& get_command_line_utilities() {
			static std::set<std::string> map;
			return map;
		}
	}

	int register_utility(const std::string& name, UtilityProgram utility, bool needs_video)
	{
		get_utility_map()[name] = utility;
		if(!needs_video) {
			get_command_line_utilities().insert(name);
		}
		return 0;
	}

	bool utility_needs_video(const std::string& name)
	{
		return get_command_line_utilities().count(name) == 0;
	}

	void run_utility(const std::string& utility_name, const std::vector<std::string>& arg)
	{
		auto it = get_utility_map().find(utility_name);
		if(it == get_utility_map().end()) {
			std::string known;
			for(UtilityMap::const_iterator i = get_utility_map().begin(); i != get_utility_map().end(); ++i) {
				if(i->second) {
					known += i->first + " ";
				}
			}
			ASSERT_LOG(false, "Unknown utility: '" << utility_name << "'; known utilities: " << known);
		}
		it->second(arg);
	}
}
