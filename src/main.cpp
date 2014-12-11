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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "lua.hpp"
#include <LuaBridge.h>

#include "action_process.hpp"
#include "ai_process.hpp"
#include "asserts.hpp"
#include "button.hpp"
#include "castles.hpp"
#include "collision_process.hpp"
#include "component.hpp"
#include "creature.hpp"
#include "dialog.hpp"
#include "draw_primitives.hpp"
#include "enet_server.hpp"
#include "engine.hpp"
#include "font.hpp"
#include "gui_elements.hpp"
#include "gui_process.hpp"
#include "hex_pathfinding.hpp"
#include "initiative_dialog.hpp"
#include "json.hpp"
#include "input_process.hpp"
#include "label.hpp"
#include "node_utils.hpp"
#include "profile_timer.hpp"
#include "random.hpp"
#include "render_process.hpp"
#include "surface.hpp"
#include "sdl_wrapper.hpp"
#include "unit_test.hpp"
#include "utility.hpp"
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

void create_world(engine& e, const std::string& world_file)
{
	try {
		e.set_map(hex::hex_map::factory(json::parse_from_file(world_file), rectf(0.0f,0.05f,0.85f,1.0f)));
	} catch(json::parse_error& pe) {
		ASSERT_LOG(false, "Error parsing " << world_file << ": " << pe.what());
	} catch(std::bad_weak_ptr& e) {
		ASSERT_LOG(false, "Bad weak ptr: " << e.what());
	}
	e.set_extents(rect(0, 0, e.get_map()->width(), e.get_map()->height()));
}

void create_gui(engine& eng)
{
	auto button_label = gui::label::create(rectf(), gui::Justify::H_CENTER | gui::Justify::V_CENTER, "End Turn", graphics::color(255,255,0), 16);
	rectf area(-0.02f,-0.02f,button_label->get_area().w()+0.05f,button_label->get_area().h()+0.02f);
	auto end_turn_button = gui::button::create(area, gui::Justify::BOTTOM | gui::Justify::RIGHT, std::bind(&engine::end_turn, &eng), button_label);
	eng.add_widget(end_turn_button);
}

COMMAND_LINE_UTILITY(server)
{
	enet::server enet_server(9000);
	enet_server.run();
}

int main(int argc, char* argv[])
{
	std::string utility_name;
	std::vector<std::string> utility_args;
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
			utility_name = arg_value;
			utility_args = std::vector<std::string>(it+1, args.end());
		}
	}

	if(!test::run_tests()) {
		// Just exit if some tests failed.
		exit(1);
	}

	if(!utility_name.empty()) {
		utility::run_utility(utility_name, utility_args);
		return 0;
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

		hex::loader();
		try {
			castle::loader(wm.get_renderer(), json::parse_from_file("data/castles.cfg"));
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/castles.cfg: " << pe.what());
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

		//create_world(e, "data/maps/map2.cfg");		// 512x512
		create_world(e, "data/maps/map4.cfg");			// 32x32
		//create_world(e, "data/maps/map5.cfg");			// 8x8
		//create_world(e, "data/maps/map6.cfg");			// 128x128
		auto g1 = e.add_entity(creature::spawn(e, p1, "goblin", point(1, 1)));
		auto g2 = e.add_entity(creature::spawn(e, b1, "flesh-golem", point(12, 13)));
		auto g3 = e.add_entity(creature::spawn(e, p1, "goblin", point(12, 12)));
		auto g4 = e.add_entity(creature::spawn(e, b1, "flesh-golem", point(6, 7)));

		create_gui(e);

 		e.add_process(std::make_shared<process::input>());
		e.add_process(std::make_shared<process::render>());
		e.add_process(std::make_shared<process::gui>());
		e.add_process(std::make_shared<process::ai>());
		e.add_process(std::make_shared<process::action>());
		// N.B. entity/map collision needs to come before entity/entity collision
		e.add_process(std::make_shared<process::em_collision>());
		e.add_process(std::make_shared<process::ee_collision>());

		auto bw = gui::initiative::create(rectf(0.0f, 0.0f, 0.4f, 0.1f), gui::Justify::H_CENTER | gui::Justify::BOTTOM);
		//bw->enable_background_rect();
		//bw->set_background_rect_color(graphics::color(255,0,255,64));
		e.add_widget(bw);

		auto info_win = gui::dialog::create(rectf(0.0f, 0.0f, 0.2f, 1.0f), gui::Justify::RIGHT | gui::Justify::TOP);
		e.add_widget(info_win);

		auto status_bar = gui::dialog::create(rectf(0.0f, 0.0f, 0.8f, 0.05f), gui::Justify::LEFT | gui::Justify::TOP);
		e.add_widget(status_bar);

		SDL_SetRenderDrawColor(wm.get_renderer(), 0, 0, 0, 255);
		while(running) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			profile::timer tm;

			SDL_RenderClear(wm.get_renderer());
			try {
				running = e.update(60.0/1000.0);
				// XXX temp
				//bw->update(e, 60.0/1000.0);
			} catch(std::bad_weak_ptr& e) {
				ASSERT_LOG(false, "Bad weak ptr: " << e.what());
			}

			draw_perf_stats(e, tm.get_time());

			// XXX temp
			//bw->draw(rect(0, 0, e.get_window().width(), e.get_window().height()), 0.0f, 1.0f);

			SDL_RenderPresent(wm.get_renderer());
	
			Uint32 delay = SDL_GetTicks() - cycle_start_tick;

			if(delay > FRAME_RATE) {
				//std::cerr << "CYCLE TOO LONG: " << delay << std::endl;
			} else {
				SDL_Delay(FRAME_RATE - delay);
			}
		}
	} catch(std::exception& ex) {
		std::cerr << ex.what();
	}
	return 0;
}
