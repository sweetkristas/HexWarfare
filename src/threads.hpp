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

#include <functional>
#include <string>
#include <thread>

#include "asserts.hpp"

namespace threading
{
    class Thread
    {
        public:
            explicit Thread(const std::string& name, std::function<int()> thread_fn)
                : thread_(thread_fn), 
				  thread_fn_(thread_fn)
            {
            }
            ~Thread() {
            }
            
            void join() {
				thread_.join();
            }
        private:
            std::thread thread_;
			std::function<int()> thread_fn_;
			
			static int thread_function(void* arg) {
				Thread* that = reinterpret_cast<Thread*>(arg);
				return that->thread_fn_();
			}
            
            Thread() = delete;
            Thread(const Thread&) = delete;
            void operator=(const Thread&) = delete;
    };
}
