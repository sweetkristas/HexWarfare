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
#include <vector>

namespace particle
{
	class particle_system_manager;

	class particle_system;
	typedef std::shared_ptr<particle_system> particle_system_ptr;
	
	class emitter;
	typedef std::shared_ptr<emitter> emitter_ptr;
	
	class affector;
	typedef std::shared_ptr<affector> affector_ptr;
}
