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

#include <SDL.h>
#include "asserts.hpp"

namespace threading
{
    class Mutex
    {
    public:
        Mutex() : mutex_(nullptr) {
            mutex_ = SDL_CreateMutex();
            ASSERT_LOG(mutex_ != nullptr, "Unable to create locking mutex");            
        }
        ~Mutex() {
            SDL_DestroyMutex(mutex_);
        }
        
        struct Lock
        {
            Lock(Mutex& mutex) : m_(mutex) {
                if(SDL_LockMutex(m_.mutex_) != 0) {
                    ASSERT_LOG(false, "Unable to lock mutex");
                }
            }
            ~Lock() {
                SDL_UnlockMutex(m_.mutex_);
            }
		private:
			Mutex& m_;
        };
    private:
        Mutex(const Mutex&) = delete;
        void operator=(const Mutex&) = delete;
        SDL_mutex* mutex_;
    };
}
