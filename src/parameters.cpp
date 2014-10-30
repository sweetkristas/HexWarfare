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

#include "asserts.hpp"
#include "parameters.hpp"
#include "random.hpp"

namespace
{
	class random_parameter : public parameter
	{
	public:
		random_parameter(const node& n) : min_(0.0f), max_(1.0f) {
			if(n.has_key("min")) {
				min_ = n["min"].as_float();
			}
			if(n.has_key("max")) {
				max_ = n["max"].as_float();
			}
		}
		float get_value(float t) {
			return generator::get_uniform_real<float>(min_, max_);
		}
	private:
		float min_;
		float max_;
	};
}

parameter::parameter(const node& n)
{
	value_ = n.as_float();
}

parameter_ptr parameter::create(const node& n)
{
	// If is numeric asume a fixed value.
	if(n.is_numeric()) {
		return std::make_shared<parameter>(n);
	}
	const std::string& type = n["type"].as_string();
	if(type == "fixed") {
		return std::make_shared<parameter>(n["value"]);
	} else if(type == "random") {
		return std::make_shared<random_parameter>(n);
	}
	ASSERT_LOG(false, "Unknown type for parameter: " << type);
	return nullptr;
}
