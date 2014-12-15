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

#include "geometry.hpp"
#include "node.hpp"

extern point node_to_point(const node& n);

class node_builder
{
public:
	template<typename T> node_builder& add(const std::string& name, const T& value)
	{
		return add_value(name, node(value));
	}

	template<typename T> node_builder& add(const std::string& name, T& value)
	{
		return add_value(name, node(value));
	}

	template<typename T> node_builder& set(const std::string& name, const T& value)
	{
		return set_value(name, node(value));
	}

	template<typename T> node_builder& set(const std::string& name, T& value)
	{
		return set_value(name, node(value));
	}

	node build();
	node_builder& clear();
private:
	node_builder& add_value(const std::string& name, const node& value);
	node_builder& set_value(const std::string& name, const node& value);

	std::map<node, std::vector<node>> attr_;
};


template<> inline node_builder& node_builder::add(const std::string& name, const node& value)
{
	return add_value(name, value);
}

template<> inline node_builder& node_builder::add(const std::string& name, node& value)
{
	return add_value(name, value);
}
