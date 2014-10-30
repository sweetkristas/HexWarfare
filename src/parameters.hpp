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

#include "node.hpp"

class parameter;
typedef std::shared_ptr<parameter> parameter_ptr;

// Base class parameter implementation, doubles as fixed value parameter.
class parameter
{
public:
	explicit parameter(float value=0) : value_(value) {}
	explicit parameter(const node& n);
	virtual ~parameter() {}
	virtual float get_value(float t) { return value_; }
	static parameter_ptr create(const node& n);
private:
	float value_;
};
