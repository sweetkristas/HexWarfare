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

#include <queue>
#include "process.hpp"
#include "units_fwd.hpp"

namespace process
{
	class input : public process
	{
	public:
		input();
		~input();
		void update(engine& eng, double t, const entity_list& elist) override;
	private:
		enum class State {
			IDLE,
			SELECT_OPPONENTS,
		} state_;
		bool handle_event(const SDL_Event& evt);
		void do_attack_message(engine& eng);
		// XXX Not sure I like all these queues of events here.
		// Need to work out if there is a better abstration to use.
		std::queue<SDL_Scancode> keys_pressed_;
		std::queue<SDL_MouseButtonEvent> mouse_button_events_;
		std::queue<SDL_MouseMotionEvent> mouse_motion_events_;
		int max_opponent_count_;
		game::unit_ptr aggressor_;
		std::vector<game::unit_ptr> targets_;
	};
}
