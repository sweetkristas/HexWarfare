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

			//auto tex = graphics::texture::get("images/noise1.png");
#include <iostream>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <curl/curl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "action_process.hpp"
#include "ai_process.hpp"
#include "asserts.hpp"
#include "button.hpp"
#include "collision_process.hpp"
#include "component.hpp"
#include "creature.hpp"
#include "dialog.hpp"
#include "engine.hpp"
#include "font.hpp"
#include "gui_elements.hpp"
#include "gui_process.hpp"
#include "hex_pathfinding.hpp"
#include "json.hpp"
#include "input_process.hpp"
#include "label.hpp"
#include "node_utils.hpp"
#include "pathfinding.hpp"
#include "profile_timer.hpp"
#include "random.hpp"
#include "render_process.hpp"
#include "surface.hpp"
#include "sdl_wrapper.hpp"
#include "unit_test.hpp"
#include "wm.hpp"

#define FRAME_RATE	(static_cast<int>(1000.0/60.0))

void sdl_gl_setup()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
}

void draw_perf_stats(engine& eng, double update_time)
{
	font::font_ptr fnt = font::get_font("SourceCodePro-Regular.ttf", 20);
	std::stringstream ss1;
	ss1 << "Frame update time (uS): " << std::fixed << update_time;
	auto surf = font::render_shaded(ss1.str(), fnt, graphics::color(1.0f, 1.0f, 0.5f), graphics::color(0, 0, 0));
	auto tex = SDL_CreateTextureFromSurface(eng.get_renderer(), surf);
	SDL_Rect dst = {0, 0, surf->w, surf->h};
	SDL_RenderCopy(eng.get_renderer(), tex, NULL, &dst);
	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
}

component_set_ptr create_world(engine& e, const std::string& world_file)
{
	component_set_ptr world = std::make_shared<component::component_set>(0);
	world->mask |= component::genmask(component::Component::MAP);
	world->mask |= component::genmask(component::Component::COLLISION);
	world->map = std::make_shared<component::mapgrid>();
	//const point& cam = e.get_camera();
	//const int screen_width_in_tiles = (e.get_window().width() + e.get_tile_size().x - 1) / e.get_tile_size().x;
	//const int screen_height_in_tiles = (e.get_window().height() + e.get_tile_size().y - 1) / e.get_tile_size().y;
	try {
		world->map->map = hex::hex_map::factory(json::parse_from_file(world_file));
	} catch(json::parse_error& pe) {
		ASSERT_LOG(false, "Error parsing " << world_file << ": " << pe.what());
	} catch(std::bad_weak_ptr& e) {
		ASSERT_LOG(false, "Bad weak ptr: " << e.what());
	}
	e.set_extents(rect(0, 0, world->map->map->width(), world->map->map->height()));
	e.add_entity(world);
	return world;
}

/*void pathfinding_test()
{
	std::vector<int> vertices{1,2,3,4,5,6,7,8};
	pathfinding::DirectedGraph<int>::GraphEdgeList edges;
	pathfinding::WeightedDirectedGraph<int,float>::EdgeWeights weights;

	edges[1] = {2,3,4};
	edges[2] = {1,4};
	edges[3] = {1,4,5};
	edges[4] = {1,2,3,5,6};
	edges[5] = {3,4,6,7,8};
	edges[6] = {4,5,8};
	edges[7] = {5,8};
	edges[8] = {5,6,7};

	for(auto& e1 : edges) {
		for(auto& e2 : e1.second) {
			weights[pathfinding::EdgePair<int>(e1.first, e2)] = 1.0f;
		}
	}

	pathfinding::DirectedGraph<int>::Pointer dg1 = std::make_shared<pathfinding::DirectedGraph<int>>(&vertices, &edges);
	pathfinding::WeightedDirectedGraph<int, float>::Pointer wg1 = std::make_shared<pathfinding::WeightedDirectedGraph<int, float>>(dg1, &weights);
	
	for(auto n : {1,2,3,4,5,6,7,8}) {
		auto res = pathfinding::path_cost_search<int,float>(wg1, n, 1.0f);
		std::cerr << "Nodes reachable from " << n << " with cost 1.0: ";
		for(auto r : res) {
			std::cerr << r << " ";
		}
		std::cerr << "\n";
	}

	auto res = pathfinding::path_cost_search<int,float>(wg1, 5, 2.0f);
	std::cerr << "Nodes reachable from " << 5 << " with cost 2.0: ";
	for(auto r : res) {
		std::cerr << r << " ";
	}
	std::cerr << "\n";

	res = pathfinding::path_cost_search<int,float>(wg1, 1, 0.5f);
	std::cerr << "Nodes reachable from " << 1 << " with cost 0.5: ";
	for(auto r : res) {
		std::cerr << r << " ";
	}
	std::cerr << "\n";
}*/

void curl_test()
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
        res = curl_easy_perform(curl);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char* argv[])
{
	std::vector<std::string> args;
	for(int i = 0; i < argc; ++i) {
		args.push_back(argv[i]);
	}

	int width = 800;
	int height = 600;
	for(auto it = args.begin(); it != args.end(); ++it) {
		size_t sep = it->find('=');
		std::string arg_name = *it;
		std::string arg_value;
		if(sep != std::string::npos) {
			arg_name = it->substr(0, sep);
			arg_value = it->substr(sep + 1);
		}

		if(arg_name == "--width") {
			width = boost::lexical_cast<int>(arg_value);
		} else if(arg_name == "--height") {
			height = boost::lexical_cast<int>(arg_value);
		} else if(arg_name == "--utility") {
			// spawn the appropriate utility.
		}
	}

	if(!test::run_tests()) {
		// Just exit if some tests failed.
		exit(1);
	}

	try {
		graphics::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
		SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "opengl", SDL_HINT_OVERRIDE);
		graphics::window_manager wm;
		sdl_gl_setup();
		wm.create_window("HexWarfare",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		wm.set_icon("images/icon.png");
		wm.gl_init();

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

		bool running = true;

		// XXX Try and load a save file here, including random seed. If no save file we generate a new seed
		generator::generate_seed();

		font::manager font_manager;
		graphics::texture::manager texture_manager(wm.get_renderer());

		try {
			creature::loader(json::parse_from_file("data/units.cfg"));
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/units.cfg: " << pe.what());
		}

		try {
			gui::section::manager gui_manager(wm.get_renderer(), json::parse_from_file("data/gui.cfg"));
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/gui.cfg: " << pe.what());
		}

		// XX engine should take the renderer as a parameter, expose it as a get function, then pass itself
		// to the update function.
		engine e(wm);
		e.set_tile_size(point(72,72));

		// Create some teams for the players
		team_ptr t1 = std::make_shared<team>(1, "Good guys");
		team_ptr t2 = std::make_shared<team>(2, "Bad guys");

		auto p1 = std::make_shared<player>(t1, PlayerType::NORMAL, "Player 1");
		e.add_player(p1);
		auto b1 = std::make_shared<player>(t2, PlayerType::AI, "Evil Bot");
		e.add_player(b1);

		//auto world = create_world(e, "data/maps/map2.cfg");		// 512x512
		auto world = create_world(e, "data/maps/map4.cfg");			// 32x32
		//auto world = create_world(e, "data/maps/map5.cfg");			// 8x8
		auto g1 = e.add_entity(creature::spawn(p1, "goblin", point(1, 1)));
		auto g2 = e.add_entity(creature::spawn(p1, "goblin", point(2, 2)));
		auto g3 = e.add_entity(creature::spawn(b1, "goblin", point(12, 12)));
		auto g4 = e.add_entity(creature::spawn(b1, "goblin", point(6, 7)));

		e.add_process(std::make_shared<process::input>());
		e.add_process(std::make_shared<process::render>());
		e.add_process(std::make_shared<process::gui>());
		e.add_process(std::make_shared<process::ai>());
		e.add_process(std::make_shared<process::action>());
		// N.B. entity/map collision needs to come before entity/entity collision
		e.add_process(std::make_shared<process::em_collision>());
		e.add_process(std::make_shared<process::ee_collision>());

		//pathfinding_test();
		curl_test();

		auto graph = hex::create_graph_from_map(world->map->map);
		auto res = hex::cost_search(graph, world->map->map->get_tile_at(g3->pos->pos.x, g3->pos->pos.y), 2.5f);
		//auto res = hex::find_available_moves(e, world->map->map, world->map->map->get_tile_at(g3->pos->pos.x, g3->pos->pos.y), 2.0f);
		std::cerr << "Found " << res.size() << " nodes\n";
		for(auto& r : res) {
			std::cerr << r->x() << "," << r->y() << "\n";
		}

		SDL_SetRenderDrawColor(wm.get_renderer(), 0, 0, 0, 255);
		while(running) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			profile::timer tm;

			SDL_RenderClear(wm.get_renderer());
			try {
				running = e.update(60.0/1000.0);
			} catch(std::bad_weak_ptr& e) {
				ASSERT_LOG(false, "Bad weak ptr: " << e.what());
			}
			SDL_SetRenderDrawColor(wm.get_renderer(), 0, 255, 0, 255);
			for(auto& r : res) {
				auto& ts = e.get_tile_size();
				point p(hex::hex_map::get_pixel_pos_from_tile_pos(r->x(), r->y()));
				SDL_Rect dest = {p.x+ts.x/4-e.get_camera().x*ts.x, p.y+ts.x/4-e.get_camera().y*ts.y, ts.x/2, ts.y/2};
				SDL_RenderSetScale(e.get_renderer(), e.get_zoom(), e.get_zoom());
				SDL_RenderFillRect(wm.get_renderer(), &dest);
				SDL_RenderSetScale(e.get_renderer(), 1.0f, 1.0f);
			}
			SDL_SetRenderDrawColor(wm.get_renderer(), 0, 0, 0, 255);
			draw_perf_stats(e, tm.get_time());
			SDL_RenderPresent(wm.get_renderer());
	
			Uint32 delay = SDL_GetTicks() - cycle_start_tick;

			if(delay > FRAME_RATE) {
				//std::cerr << "CYCLE TOO LONG: " << delay << std::endl;
			} else {
				SDL_Delay(FRAME_RATE - delay);
			}
		}
	} catch(std::exception& e) {
		std::cerr << e.what();
	}
	return 0;
}
