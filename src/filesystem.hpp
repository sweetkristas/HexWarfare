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

#include <map>

namespace sys
{
	typedef std::map<std::string, std::string> file_path_map;

	bool file_exists(const std::string& name);
	std::string read_file(const std::string& name);
	void write_file(const std::string& name, const std::string& data);
	void get_unique_files(const std::string& path, file_path_map& fpm);
}
