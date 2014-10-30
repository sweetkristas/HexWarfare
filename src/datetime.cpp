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

#include <iomanip>
#include <sstream>

#include "asserts.hpp"
#include "datetime.hpp"

namespace
{
	const int TURNS_PER_DAY = 3000;

	const int DAYS_PER_YEAR = 200;
	const int MONTHS_PER_YEAR = 10;
	const int HOURS_PER_DAY = 20;
	const int MINUTES_PER_HOUR = 10;
	const int SECONDS_PER_HOUR = 15 * MINUTES_PER_HOUR;
	const int TURNS_PER_YEAR = DAYS_PER_YEAR * TURNS_PER_DAY;
	const int TURNS_PER_HOUR = TURNS_PER_DAY / HOURS_PER_DAY;
	const int TURNS_PER_MINUTE = TURNS_PER_HOUR / MINUTES_PER_HOUR;
	const int TURNS_PER_SECOND = TURNS_PER_HOUR / SECONDS_PER_HOUR;
	const int SUNRISE = 600;
	const int SUNSET = TURNS_PER_DAY - 600;

	const int start_offset = SUNRISE;
	const int year_offset = 454;

	const int days_per_month[MONTHS_PER_YEAR] = {
		20, 19, 21, 20, 20, 19, 19, 21, 21, 20,
	};
	const int cumulative_turns_per_month[MONTHS_PER_YEAR+1] = {
		0,
		20 * TURNS_PER_DAY,
		39 * TURNS_PER_DAY,
		60 * TURNS_PER_DAY,
		80 * TURNS_PER_DAY,
		100 * TURNS_PER_DAY,
		119 * TURNS_PER_DAY,
		138 * TURNS_PER_DAY,
		159 * TURNS_PER_DAY,
		180 * TURNS_PER_DAY,
		200 * TURNS_PER_DAY,
	};

	const int DAY_PHASES = 10;
	const int day_phases[DAY_PHASES] = {150, 550, 650, 1400, 1500, 2250, 2350, 2550, 2850, 3000};
	const std::string phases_of_the_day[DAY_PHASES] = {
		"Midnight",
		"Early Morning",
		"Dawn",
		"Mid-morning",
		"Noon",
		"Afternoon",
		"Dusk",
		"Evening",
		"Night",
		"Midnight",
	};


	const std::string month_names[MONTHS_PER_YEAR] = {
		"Jujalst",
		"Ibasis",
		"Keb",
		"Qrelm",
		"Eduth",
		"Greiq",
		"Qleb",
		"Vuwak",
		"Xeps",
		"Drajans",
	};
}

datetime::datetime(int turns)
	: turns_(turns + start_offset)
{
}

int datetime::get_hour()
{
	return (turns_ % TURNS_PER_DAY) / TURNS_PER_HOUR;
}

int datetime::get_minute()
{
	return (turns_ % TURNS_PER_MINUTE);
}

int datetime::get_second()
{
	return (turns_ % TURNS_PER_SECOND);
}

int datetime::get_day()
{
	return (((turns_ % TURNS_PER_YEAR) - cumulative_turns_per_month[get_month() - 1]) / TURNS_PER_DAY) + 1;
}

int datetime::get_month()
{
	int mnth = (turns_ % TURNS_PER_YEAR);
	for(int n = 0; n < MONTHS_PER_YEAR+1; ++n) {
		if(mnth < cumulative_turns_per_month[n]) {
			return n;
		}
	}
	ASSERT_LOG(false, "Error in date conversion logic while calculating month: " << turns_ << "," << mnth);
}

int datetime::get_year()
{
	return year_offset + turns_ / TURNS_PER_YEAR;
}

int datetime::is_day()
{
	int t = turns_ % TURNS_PER_DAY;
	return t < SUNRISE || t > SUNSET ? false : true;
}

int datetime::is_night()
{
	return !is_day();
}

int datetime::time_of_day_index()
{
	int tod = turns_ % TURNS_PER_DAY;
	for(int n = 0; n != DAY_PHASES; ++n) {
		if(tod <= day_phases[n]) {
			return n;
		}
	}
	ASSERT_LOG(false, "Error in date conversion logic while calculating time of day: " << turns_ << "," << tod);
}

std::string datetime::printable()
{
	std::stringstream ss1;
	ss1 << std::setfill('0') << std::setw(2) << get_day()
		<< "/"
		<< month_names[get_month() - 1]
		<< "/"
		<< std::setw(4) << get_year()
		<< " "
		<< std::setw(2) << get_hour()
		<< ":"
		<< std::setw(2) << get_minute()
		<< " "
		<< phases_of_the_day[time_of_day_index()]
		;
	return ss1.str();
}
