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
#include <condition_variable>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>

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
			std::unique_lock<std::mutex> lock(guard_);
			q_.push(data);
			lock.unlock();
			cv_.notify_one();
		}

		bool empty() const
		{
			std::unique_lock<std::mutex> lock(guard_);
			return q_.empty();
		}

		bool try_pop(T& popped_value)
		{
			std::unique_lock<std::mutex> lock(guard_);
			if(q_.empty()) {
				return false;
			}
        
			popped_value = q_.front();
			q_.pop();
			return true;
		}

		bool wait_and_pop(T& popped_value, int64_t interval = 0)
		{
			using namespace std::chrono;
			std::unique_lock<std::mutex> lock(guard_);
			system_clock::time_point time_limit = system_clock::now() + milliseconds(interval);
			while(q_.empty()) {
				if(interval) {
					if(cv_.wait_until<system_clock, system_clock::duration>(lock, time_limit) == std::cv_status::timeout) {
						return false;
					}
				} else {
					cv_.wait(lock);
				}
			}
        
			popped_value = q_.front();
			q_.pop();
			return true;
		}

	private:
		std::queue<T> q_;
		mutable std::mutex guard_;
		std::condition_variable cv_;
		queue(const queue&) = delete;
		void operator=(const queue&) = delete;
	};
}
