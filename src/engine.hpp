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

#include "engine_fwd.hpp"
#include "game_state.hpp"
#include "geometry.hpp"
#include "hex_fwd.hpp"
#include "network_server.hpp"
#include "particles.hpp"
#include "process.hpp"
#include "profile_timer.hpp"
#include "property_animate.hpp"
#include "quadtree.hpp"
#include "widget.hpp"
#include "wm.hpp"

enum class EngineState {
	PLAY,
	PAUSE,
	GAME_OVER,
	QUIT,
};

enum class EngineUserEvents {
	NEW_TURN = 1,
};

class engine
{
public:
	engine(game::state& game_state, graphics::window_manager& wm);
	~engine();
	
	component_set_ptr add_entity(component_set_ptr e);
	void remove_entity(component_set_ptr e);

	void add_process(process::process_ptr s);
	void remove_process(process::process_ptr s);

	graphics::window_manager& get_window() { return wm_; }
	const graphics::window_manager& get_window() const { return wm_; }

	SDL_Renderer* get_renderer() { return wm_.get_renderer(); }
	const SDL_Renderer* get_renderer() const { return wm_.get_renderer(); }

	void set_state(EngineState state) { state_ = state; }
	EngineState get_state() const { return state_; }

	void end_turn();

	bool update(double time);

	void set_extents(const rect& extents);
	const rect& get_extents() const;

	void set_camera(const point& cam);
	void set_camera(int x, int y);
	const point& get_camera() { return camera_; }

	void set_camera_scale(int scale) { camera_scale_ = scale; }
	int get_camera_scale() const { return camera_scale_; }
	float get_zoom() const;

	particle::particle_system_manager& get_particles() { return particles_; }

	const entity_list& get_entities() const { return entity_list_; }

	const point& get_tile_size() const { return tile_size_; }
	void set_tile_size(const point& p) { tile_size_ = p; }

	hex::hex_map_ptr get_map() const { return map_; }
	void set_map(hex::hex_map_ptr map) { map_ = map; }

	void add_widget(gui::widget_ptr w) { widgets_.emplace_back(w); }
	const std::vector<gui::widget_ptr>& get_widgets() const { return widgets_; }

	game::state& get_game_state() { return game_state_; }
	const game::state& get_game_state() const { return game_state_; }

	network::client_weak_ptr get_netclient() const { return client_; }
	void set_netclient(network::client_weak_ptr c) { client_ = c; }

	void add_animated_property(const std::string& name, property::animate_ptr a);

	void process_update(game::Update* up);

	void set_active_player(player_ptr p) { active_player_ = p; }
	const player_ptr& get_active_player() const { return active_player_; }

	// Convenience funtion to create a spite/position entity and add it, from a string
	component_set_ptr create_entity_from_string(const std::string& s);

	component_set_ptr get_entity_for_unit_uuid(const uuid::uuid& id) const;
private:
	game::state& game_state_;
	EngineState state_;
	point camera_;
	unsigned camera_scale_;
	graphics::window_manager& wm_;
	entity_list entity_list_;
	std::vector<process::process_ptr> process_list_;
	point tile_size_;
	rect extents_;
	particle::particle_system_manager particles_;
	hex::hex_map_ptr map_;
	std::vector<gui::widget_ptr> widgets_;
	network::client_weak_ptr client_;
	property::manager property_manager_;
	player_ptr active_player_;

	void translate_mouse_coords(SDL_Event* evt);
	void process_events();

	void clip_camera_to_extents();

	void entity_health_check();

	engine() = delete;
	engine(const engine&) = delete;
	void operator=(const engine&) = delete;
};
