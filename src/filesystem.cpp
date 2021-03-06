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

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <boost/filesystem.hpp>

#include "asserts.hpp"
#include "filesystem.hpp"

namespace sys
{
	using namespace boost::filesystem;

	bool file_exists(const std::string& name)
	{
		path p(name);
		return exists(p) && is_regular_file(p);
	}

	std::string read_file(const std::string& name)
	{
		ASSERT_LOG(file_exists(name), "Couldn't read file: " << name);
		std::ifstream file(name, std::ios_base::binary);
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	void write_file(const std::string& name, const std::string& data)
	{
		path p(name);
		ASSERT_LOG(p.is_absolute() == false, "Won't write absolute paths: " << name);
		ASSERT_LOG(p.has_filename(), "No filename found in write_file path: " << name);

		// Create any needed directories
		create_directories(p);

		// Write the file.
		std::ofstream file(name, std::ios_base::binary);
		file << data;
	}

	void get_unique_files(const std::string& name, file_path_map& fpm)
	{
		path p(name);
		if(exists(p)) {
			ASSERT_LOG(is_directory(p) || is_other(p), "get_unique_files() not directory: " << name);
			for(auto it = directory_iterator(p); it != directory_iterator(); ++it) {
				if(is_regular_file(it->path())) {
					fpm[it->path().filename().generic_string()] = it->path().generic_string();
				} else {
					get_unique_files(it->path().generic_string(), fpm);
				}
			}
		} else {
			std::cerr << "WARNING: path " << p.generic_string() << " doesn't exit" << std::endl;
		}
	}
}