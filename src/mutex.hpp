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

// C++ wrapper around SDL mutex services

#include <mutex>

#include "asserts.hpp"

namespace threading
{
    class Mutex
    {
    public:
        Mutex() : mutex_() {}
        ~Mutex() {}
        
        struct Lock
        {
            Lock(Mutex& mutex) : lock_(mutex.mutex_) {
				lock_.lock();
            }
            ~Lock() {
                lock_.unlock();
            }
		private:
			std::unique_lock<std::mutex> lock_;
        };
    private:
        Mutex(const Mutex&) = delete;
        void operator=(const Mutex&) = delete;
        std::mutex mutex_;
    };
}
