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

#include <cstdint>
#include <queue>
#include <boost/thread.hpp>
#include <boost/chrono/chrono.hpp>

namespace queue 
{
	template<class T>
	class queue
	{
	public:
		queue()
		{}
		void push(T const& data)
		{
			boost::mutex::scoped_lock lock(guard_);
			q_.push(std::move(data));
			lock.unlock();
			cv_.notify_one();
		}

		bool empty() const
		{
			boost::mutex::scoped_lock lock(guard_);
			return q_.empty();
		}

		bool try_pop(T& popped_value)
		{
			boost::mutex::scoped_lock lock(guard_);
			if(q_.empty()) {
				return false;
			}
        
			popped_value = q_.front();
			q_.pop();
			return true;
		}

		bool wait_and_pop(T& popped_value, int64_t interval = 0)
		{
			using namespace boost::chrono;
			boost::mutex::scoped_lock lock(guard_);
			system_clock::time_point time_limit = system_clock::now() + milliseconds(interval);
			while(q_.empty()) {
				if(interval) {
					if(cv_.wait_until<system_clock, system_clock::duration>(lock, time_limit) == boost::cv_status::timeout) {
						return false;
					}
				} else {
					cv_.wait(lock);
				}
			}
        
			popped_value = std::move(q_.front());
			q_.pop();
			return true;
		}

	private:
		std::queue<T> q_;
		mutable boost::mutex guard_;
		boost::condition_variable cv_;
		queue(const queue&) = delete;
		void operator=(const queue&) = delete;
	};
}
