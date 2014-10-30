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

#include <cstdint>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>

struct formatter
{
	template<typename T>
	formatter& operator<<(const T& o) {
		stream << o;
		return *this;
	}

	const std::string str() {
		return stream.str();
	}

	const char* c_str() {
		return str().c_str();
	}

	operator std::string() {
		return stream.str();
	}
	std::ostringstream stream;
};

template<> inline formatter& formatter::operator<<(const std::vector<uint8_t>& o) {
	for(auto c : o) {
		if(c < 32 || c > 127) {
			stream << "[" << std::setw(2) << std::setfill('0') << std::hex << int(c) << std::dec << "]";
		} else {
			stream << char(c);
		}
	}
	return *this;
}
