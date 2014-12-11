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

#pragma once

#include <functional>
#include <vector>

#define UTILITY(name) \
    void UTILITY_##name(const std::vector<std::string>& args); \
	static int UTILITY_VAR_##name = utility::register_utility(#name, UTILITY_##name, true); \
	void UTILITY_##name(const std::vector<std::string>& args)

#define COMMAND_LINE_UTILITY(name) \
    void UTILITY_##name(const std::vector<std::string>& args); \
	static int UTILITY_VAR_##name = utility::register_utility(#name, UTILITY_##name, false); \
	void UTILITY_##name(const std::vector<std::string>& args)

namespace utility
{
	typedef std::function<void (const std::vector<std::string>&)> UtilityProgram;

	int register_utility(const std::string& name, UtilityProgram utility, bool needs_video);

	void run_utility(const std::string& utility_name, const std::vector<std::string>& arg);
}
