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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/random/mersenne_twister.hpp>

#include <iostream> 
#include <sstream>

#include "asserts.hpp"
#include "uuid.hpp"

namespace uuid
{
	namespace 
	{
		boost::mt19937* twister_rng() 
		{
			static boost::mt19937 ran;
			ran.seed(static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds()));
			return &ran;
		}
	}

	boost::uuids::uuid generate() 
	{
		static boost::uuids::basic_random_generator<boost::mt19937> gen(twister_rng());
		return gen();
	}

	std::string write(const boost::uuids::uuid& id) 
	{
		std::stringstream str;
		str.fill('0');
		str.width(2);
		str.flags(std::ios::hex);
		for(auto num : id) {
			str << static_cast<int>(num);
		}
		return str.str();
	}

	boost::uuids::uuid read(const std::string& s) 
	{
		boost::uuids::uuid result;

		const std::string& nums = s;
		const char* ptr = nums.c_str();
		ASSERT_LOG(nums.size() == 32, "Trying to deserialize bad UUID: " << nums);
		for(auto itor = result.begin(); itor != result.end(); ++itor) {
			char buf[3];
			buf[0] = *ptr++;
			buf[1] = *ptr++;
			buf[2] = 0;

			*itor = static_cast<uint8_t>(strtol(buf, NULL, 16));
		}

		return result;
	}
}

std::ostream& operator<<(std::ostream& os, const uuid::uuid& uid)
{
	os << uuid::write(uid);
	return os;
}
