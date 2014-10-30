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

#include <algorithm>
#include <limits>

#include "particles.hpp"
#include "random.hpp"

namespace particle
{
	particle_system_manager::particle_system_manager(SDL_Renderer* renderer)
		: renderer_(renderer)
	{
	}

	void particle_system_manager::add_system(particle_system_ptr ps)
	{
		ps_list_.emplace_back(ps);
	}

	void particle_system_manager::update(float t)
	{
		for(auto& ps : ps_list_) {
			ps->update(t);
		}
		// Remove any stopped systems.
		ps_list_.erase(std::remove_if(ps_list_.begin(), ps_list_.end(), [](particle_system_ptr p) { 
				return p->is_stopped(); 
			}), ps_list_.end());
	}

	void particle_system_manager::draw() const
	{
		for(auto& ps : ps_list_) {
			ps->draw(renderer_);
		}
	}

	particle_system::particle_system(const point& pos, const node& n)
		: position_(pos), 
		  running_(true),
		  lifetime_(std::numeric_limits<float>::max())
	{
		emitter_ = emitter::create(pos, n["emitter"]);
		if(n.has_key("affector")) {
			affector_ = affector::create(n["affector"]);
		}
		if(n.has_key("lifetime")) {
			lifetime_ = parameter::create(n["lifetime"])->get_value(0);
		}
	}

	void particle_system::update(float t)
	{
		lifetime_ -= t;
		if(lifetime_ < 0.0f) {
			running_ = false;
		} else {
			emitter_->update(t);
			if(affector_) {
				affector_->update(t, emitter_);
			}

			for(auto& p : *emitter_) {
				// update particle positions
				p.pos += p.direction * t;
			}
		}

	}

	void particle_system::draw(SDL_Renderer* renderer) const
	{
		// Note that we're not using color, texture or size here, which is unfortunate really.
		std::vector<SDL_Point> points;
		points.reserve(emitter_->size());
		for(auto& p : *emitter_) {
			SDL_Point sp = {static_cast<int>(p.pos.x), static_cast<int>(p.pos.y)};
			points.emplace_back(sp);
		}
		if(!points.empty()) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawPoints(renderer, &points[0], points.size());
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		}
	}

	particle_system_ptr particle_system::create(const point& pos, const node& n)
	{
		return std::make_shared<particle_system>(pos, n);
	}

	namespace
	{
		class point_emitter : public emitter
		{
		public:
			explicit point_emitter(const point& pos, const node& n) : emitter(pos, n) {}
		private:
			void handle_create_particle(particle* p, float t) override { /* nothing to do. */ }
		};

		class square_emitter : public emitter
		{
		public:
			explicit square_emitter(const point& pos, const node& n) 
				: emitter(pos, n),
				  dims_(1.0f, 1.0f)
			{
				if(n.has_key("width")) {
					dims_.x = n["width"].as_float();
				} else if(n.has_key("square_width")) {
					dims_.x = n["square_width"].as_float();
				}
				if(n.has_key("height")) {
					dims_.y = n["height"].as_float();
				} else if(n.has_key("square_height")) {
					dims_.y = n["square_height"].as_float();
				}
				if(n.has_key("dimensions") && n["dimensions"].is_list() && n["dimensions"].num_elements() == 2) {
					dims_.x = n["dimensions"][0].as_float();
					dims_.y = n["dimensions"][1].as_float();
				}
			}
		private:
			void handle_create_particle(particle* p, float t) override {
				p->pos.x += generator::get_uniform_real<float>(0.0f, dims_.x) - dims_.x / 2;
				p->pos.y += generator::get_uniform_real<float>(0.0f, dims_.y) - dims_.y / 2;
			}
			// Dimensions of the square
			pointf dims_;
		};

		class circle_emitter : public emitter
		{
		public:
			explicit circle_emitter(const point& pos, const node& n)
				: emitter(pos, n), radius_(1.0f), circle_step_(0.1f), circle_angle_(0.0f), emit_random_(false)
			{
				if(n.has_key("circle_radius")) {
					radius_ = n["circle_radius"].as_float();
				}
				if(n.has_key("circle_step")) {
					circle_step_ = n["circle_step"].as_float();
				}
				if(n.has_key("circle_angle")) {
					circle_angle_ = n["circle_angle"].as_float();
				}
				if(n.has_key("emit_random")) {
					emit_random_ = n["emit_random"].as_bool();
				}
			}
		private:
			void handle_create_particle(particle* p, float t) override {
				float angle = 0.0f;
				if(emit_random_) {
					angle = generator::get_uniform_real<float>(0.0f, static_cast<float>(2.0 * M_PI));
				} else {
					angle = t * circle_step_;
				}
				p->pos.x += radius_ * sin(angle + circle_angle_);
				p->pos.y += radius_ * cos(angle + circle_angle_);
			};
			float radius_;
			float circle_step_;
			float circle_angle_;
			bool emit_random_;
		};
	}

	emitter::emitter(const point& pos, const node& n)
		: max_particles_(100),
		  particle_direction_(0.0f,1.0f,0.0f),
		  particle_velocity_(10.0f),
		  accumulated_rate_time_(0.0f),
		  accumulated_time_(0.0f),
		  pos_(pos)
	{
		if(n.has_key("max_particles")) {
			max_particles_ = static_cast<int>(n["max_particles"].as_int());
		}
		if(n.has_key("rate")) {
			rate_ = parameter::create(n["rate"]);
		} else {
			rate_ = std::make_shared<parameter>(20.0f);
		}
		if(n.has_key("particle_lifetime")) {
			particle_lifetime_ = parameter::create(n["particle_lifetime"]);
		} else {
			particle_lifetime_ = std::make_shared<parameter>(5.0f);
		}
		if(n.has_key("particle_size")) {
			particle_size_ = parameter::create(n["particle_size"]);
		} else {
			particle_size_ = std::make_shared<parameter>(1.0f);
		}
		if(n.has_key("particle_direction")) {
			const auto& dir = n["particle_direction"];
			ASSERT_LOG(dir.is_list() && (dir.num_elements() == 2 || dir.num_elements() == 3),
				"Attribute 'particle_direction' should be a two or three element list.");
			particle_direction_.x = dir[0].as_float();
			particle_direction_.y = dir[1].as_float();
			particle_direction_.z = dir.num_elements() == 3 ? dir[2].as_float() : 0.0f;
		}
		if(n.has_key("particle_velocity")) {
			particle_velocity_ = n["particle_velocity"].as_float();
		}
		// color
		// orientation
	}

	void emitter::update(float t)
	{
		// Create some new particles if needed.
		if(particles_.size() < static_cast<unsigned>(max_particles_)) {
			int pcount = static_cast<int>(rate_->get_value(accumulated_time_) * (t + accumulated_rate_time_));
			if(pcount > 0) {				
				pcount = std::min<int>(max_particles_, static_cast<int>(particles_.size()) + pcount);
				accumulated_rate_time_ -= static_cast<float>(pcount);
				for(int n = 0; n != pcount; ++n) {
					particles_.emplace_back(particle());
					particle& p = particles_.back();
					p.color = graphics::color(255, 255, 255);
					p.direction = particle_direction_;
					p.life = particle_lifetime_->get_value(0);
					p.size = particle_size_->get_value(0);
					p.pos = glm::vec3(static_cast<float>(pos_.x), static_cast<float>(pos_.y), 0.0f);
					p.velocity = particle_velocity_;
					handle_create_particle(&p, t);
					p.old_pos = p.pos;
				}
			} else {
				accumulated_rate_time_ += t;
			}
		}
		handle_update(t);
		accumulated_time_ += t;

		// decrement life-time on particles and remove any that have expired.
		for(auto& p : particles_) {
			p.life -= t;
		}
		particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](particle& p){
			return p.life <= 0.0f;
		}), particles_.end());
	}

	emitter_ptr emitter::create(const point&pos, const node& n)
	{
		const std::string& type = n["type"].as_string();
		if(type == "point") {
			return std::make_shared<point_emitter>(pos, n);
		} else if(type == "square") {
			return std::make_shared<square_emitter>(pos, n);
		} else if(type == "circle") {
			return std::make_shared<circle_emitter>(pos, n);
		}
		ASSERT_LOG(false, "Unknown emitter type: " << type);
		return nullptr;
	}

	affector::affector(const node& n)
	{
	}

	affector_ptr affector::create(const node& n)
	{
		return nullptr;
	}
}
