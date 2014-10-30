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

#include <tuple>

#include "SDL.h"
#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>

#include "color.hpp"
#include "geometry.hpp"
#include "node.hpp"
#include "parameters.hpp"
#include "particles_fwd.hpp"
#include "texture.hpp"

// Simplified particle system.
namespace particle
{
	// Basic definition of a particle, which has position
	struct particle
	{
		glm::vec3 pos;
		glm::vec3 old_pos;
		float velocity;
		glm::vec3 direction;
		graphics::color color;
		float size;
		float life;
	};

	class emitter
	{
	public:
		typedef std::vector<particle>::iterator iterator;
		typedef std::vector<particle>::const_iterator const_iterator;

		explicit emitter(const point& pos, const node& n);
		virtual ~emitter() {}

		void update(float t);

		const point& get_position() const { return pos_; }
		void set_position(const point& p) { pos_ = p; }

		parameter_ptr get_particle_lifetime() { return particle_lifetime_; }
		parameter_ptr get_particle_size() { return particle_size_; }

		float get_accumulated_time() const { return accumulated_time_; }

		// Ranged based for-loop support.
		iterator begin() { return particles_.begin(); }
		const_iterator begin() const { return particles_.begin(); }
		iterator end() { return particles_.end(); }
		const_iterator end() const { return particles_.end(); }
		size_t size() { return particles_.size(); }

		static emitter_ptr create(const point& pos, const node& n);
	private:
		virtual void handle_update(float t) {}
		virtual void handle_create_particle(particle* p, float t) = 0;
		// max number of particles
		int max_particles_;
		parameter_ptr particle_lifetime_;
		parameter_ptr particle_size_;
		glm::vec3 particle_direction_;
		float particle_velocity_;
		// Rate of particle creation, in particles/second
		parameter_ptr rate_;
		// So we can track particle creation rates
		float accumulated_rate_time_;
		float accumulated_time_;
		// Position of the emitter.
		point pos_;
		std::vector<particle> particles_;
	};

	class affector
	{
	public:
		explicit affector(const node& n);
		virtual ~affector() {}

		virtual void update(float t, emitter_ptr e) = 0;
		static affector_ptr create(const node& n);
	private:
	};

	class particle_system
	{
	public:
		explicit particle_system(const point& pos, const node& n);
		// Calling the stop method will stop an active particle system and
		// schedule it to be removed.
		void stop() { running_ = false; }
		// Called to update the particles for rendering.
		void update(float t);
		// Called to draw the particles.
		void draw(SDL_Renderer* renderer) const;
		bool is_stopped() const { return !running_; }
		static particle_system_ptr create(const point& pos, const node& n);
	private:
		point position_;
		bool running_;
		// These could be lists etc, but we're trying to keep things simple
		emitter_ptr emitter_;
		affector_ptr affector_;
		// Lifetime of particle system
		float lifetime_;
		//texture tex_;
		particle_system();
	};

	class particle_system_manager
	{
	public:
		explicit particle_system_manager(SDL_Renderer* renderer);
		void add_system(particle_system_ptr ps);
		void update(float t);
		void draw() const;
	private:
		std::vector<particle_system_ptr> ps_list_;
		SDL_Renderer* renderer_;
	};
}
