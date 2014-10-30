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

#include "node_utils.hpp"

node_builder& node_builder::add_value(const std::string& name, const node& value)
{
	attr_[node(name)].emplace_back(value);
	return *this;
}

node_builder& node_builder::set_value(const std::string& name, const node& value)
{
	node key(name);
	attr_.erase(key);
	attr_[key].emplace_back(value);
	return *this;
}

node node_builder::build()
{
	std::map<node, node> res;
	for(auto& i : attr_) {
		if(i.second.size() == 1) {
			res[i.first] = i.second[0];
		} else {
			res[i.first] = node(&i.second);
		}
	}
	return node(&res);
}

node_builder& node_builder::clear()
{
	attr_.clear();
	return *this;
}
