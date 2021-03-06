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

#include "process.hpp"

namespace process
{
	class ee_collision : public process
	{
	public:
		ee_collision();
		~ee_collision();
		void update(engine& eng, double t, const entity_list& elist) override;
	};

	class em_collision : public process
	{
	public:
		em_collision();
		~em_collision();
		void update(engine& eng, double t, const entity_list& elist) override;
	};
}
