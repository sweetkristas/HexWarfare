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

#include "SDL.h"

#include "engine_fwd.hpp"

namespace process
{
	enum class ProcessPriority {
		input			= 100,
		ai				= 200,
		collision		= 600,
		action			= 700,
		world			= 800,
		gui			    = 850,
		render			= 900
	};

	// abstract system interface
	class process
	{
	public:
		explicit process(ProcessPriority priority);
		virtual ~process();
		virtual void start() {}
		virtual void end() {}
		virtual void update(engine& eng, double t, const entity_list& elist) = 0;

		bool process_event(const SDL_Event& evt);
		ProcessPriority get_priority() const { return priority_; }
	private:
		process();
		virtual bool handle_event(const SDL_Event& evt) { return false; }
		ProcessPriority priority_;
	};

	typedef std::shared_ptr<process> process_ptr;
}
