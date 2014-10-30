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

#include <string>

// Get in-game time. We let game-time increment at a rate of 3000 turns per day, split
// at dat 1800, night 1200. At least initially anyway. At the automatic turn rate of
// one turn per 200mS(real time) it gives a day length of 10 minutes. 6 minutes of day
// and 4 minutes night (in real time).

class datetime
{
public:
	datetime(int turns);
	
	int get_hour();
	int get_minute();
	int get_second();

	int get_day();
	int get_month();
	int get_year();
	
	int is_day();
	int is_night();

	std::string printable();
private:
	int time_of_day_index();
	int turns_;
};
