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

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace property
{
	class animate_base
	{
	public:
		animate_base() {}
		virtual ~animate_base() {}
		bool process(double t) 
		{
			return handle_process(t);
		}
	private:
		virtual bool handle_process(double t) = 0;
	};
	typedef std::shared_ptr<animate_base> animate_ptr;

	template<typename T, typename D>
	class animate : public animate_base
	{
	public:
		typedef std::function<D(T t, T d)> easing_fn;
		typedef std::function<void(const D&)> mutate_fn;
		animate(easing_fn e, mutate_fn m, T duration) 
			: fn_(e),
			  mutate_fn_(m),
			  duration_(duration),
			  acc_(0)
		{
		}
	private:
		bool handle_process(double t) override
		{
			acc_ += t;
			mutate_fn_(fn_(acc_, duration_));
			if(acc_ > duration_) {
				return false;
			}
			return true;
		}

		easing_fn fn_;
		mutate_fn mutate_fn_;
		T duration_;
		T acc_;
	};

	class manager
	{
	public:
		manager() {}
		void add(const std::string& qname, animate_ptr a)
		{
			queue_[qname].emplace_back(a);
		}
		void process(double t)
		{
			for(auto it = queue_.begin(); it != queue_.end(); ) {
				if(!it->second.empty()) {
					auto& a = it->second.front();
					if(!a->process(t)) {
						it->second.pop_front();
						if(it->second.empty()) {
							queue_.erase(it++);
						} else {
							++it;
						}
					} else {
						++it;
					}
				} else {
					queue_.erase(it++);
				}
			}
		}
	private:
		std::map<std::string, std::deque<animate_ptr>> queue_;
	};
}
