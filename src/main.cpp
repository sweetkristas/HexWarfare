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
#include <thread>
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
#include "bot.hpp"
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
#include "game_state.hpp"
#include "gui_elements.hpp"
#include "gui_process.hpp"
#include "hex_pathfinding.hpp"
#include "initiative_dialog.hpp"
#include "internal_server.hpp"
#include "internal_client.hpp"
#include "json.hpp"
#include "input_process.hpp"
#include "label.hpp"
#include "network_server.hpp"
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
		// parse the world file
		auto n = json::parse_from_file(world_file);
		// create a logical version of the world
		auto lmap = hex::logical::map::factory(n);
		e.set_map(hex::hex_map::factory(lmap, n, rectf(0.0f,0.05f,0.85f,1.0f)));
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

void load_scenario(engine& eng, const std::string& name)
{
	auto& gs = eng.get_game_state();
	try {
		auto scen = json::parse_from_file(name);
		ASSERT_LOG(scen.is_map(), "Scenario must be a map, got: " << scen.type_as_string());
		ASSERT_LOG(scen.has_key("name") && scen.has_key("map") && scen.has_key("starting_units"), 
			"Scenario file must have 'name', 'map' and 'starting_units' attributes.");
		std::cerr << "Loading scenario " << scen["name"].as_string() << " from " << name << "\n";
		if(scen.has_key("max_players")) {
			if(gs.get_player_count() > scen["max_players"].as_int()) {
				ASSERT_LOG(false, "Unable to load scenario number of players in game is greater than the maximum number allowed.");
			}
		}
		create_world(eng, "data/" + scen["map"].as_string());
		for(auto& c : scen["starting_units"].as_list()) {
			ASSERT_LOG(c.has_key("name"), "In 'starting_units' list, you must provide a 'name' attribute.");
			ASSERT_LOG(c.has_key("location"), "In 'starting_units' list, you must provide a 'location' attribute.");
			ASSERT_LOG(c.has_key("player"), "In 'starting_units' list, you must provide a 'player' attribute.");
			// N.B. Don't really like this way of matching player in scenario file to player instance.
			// XXX should come up with a better way.
			int player_id = c["player"].as_int32();
			eng.add_entity(creature::spawn(gs, gs.get_player_by_id(player_id), c["name"].as_string(), node_to_point(c["location"])));
		}
	} catch(json::parse_error& pe) {
		ASSERT_LOG(false, "Error parsing " << name << ": " << pe.what());
	}
	gs.set_map(eng.get_map()->get_logical_map());
}

void local_server_code(game::state gs, network::server_ptr server)
{
	game::Update* up;
	bool running = true;

	// create and send a start game packet.
	up = gs.create_update();
	up->set_game_start(true);
	server->write_send_queue(up);
	server->process();

	while(running) {
		if((up = server->read_recv_queue()) != nullptr) {
			std::cerr << "local_server_code: Got message: " << up->id() << "\n";
			// XXX do more processing here.
			game::Update* nup = gs.validate_and_apply(up);
			if(nup) {
				server->write_send_queue(nup);
			}
			if(up->has_quit() && up->quit() && up->id() == -1) {
				running = false;
			}
			delete up;

			server->process();
		}
	}
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

	std::string scenario_file("data/scenario/scenario1.cfg");

	bool local_server = true;
	std::string server_name = "localhost";
	int server_port = 9000;
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
			if(arg_value == "server") {
				local_server = false; 
			}
			utility_name = arg_value;
			utility_args = std::vector<std::string>(it+1, args.end());
		} else if(arg_name == "--scenario") {
			// XXX A proper implementation searches for the file matching scenario_file, and checks
			// for whether .cfg is already specified.
			scenario_file = "data/scenario/" + arg_value + ".cfg";
		}
	}
	
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

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
		//SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "opengl", SDL_HINT_OVERRIDE);
		graphics::window_manager wm;
		//sdl_gl_setup();
		wm.create_window("HexWarfare",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			width,
			height,
			/*SDL_WINDOW_OPENGL |*/ SDL_WINDOW_RESIZABLE);
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

		try {
			hex::loader(json::parse_from_file("data/hex_tiles.cfg"));
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/hex_tiles.cfg: " << pe.what());
		}

		try {
			castle::loader(wm.get_renderer(), json::parse_from_file("data/castles.cfg"));
		} catch(json::parse_error& pe) {
			ASSERT_LOG(false, "Error parsing data/castles.cfg: " << pe.what());
		}

		game::state gs;
		// XX engine should take the renderer as a parameter, expose it as a get function, then pass itself
		// to the update function.
		engine e(gs, wm);
		e.set_tile_size(point(72,72));

		// Create some teams for the players
		team_ptr t1 = gs.create_team_instance("Good guys");
		team_ptr t2 = gs.create_team_instance("Bad guys");

		auto p1 = std::make_shared<player>(t1, PlayerType::NORMAL, "Player 1");
		p1->set_id(0);
		gs.add_player(p1);
		auto b1 = std::make_shared<ai::bot>(t2, "Evil Bot");
		b1->set_id(1);
		gs.add_player(b1);

		load_scenario(e, scenario_file);

		create_gui(e);

		std::unique_ptr<std::thread> local_server_thread;
		std::unique_ptr<std::thread> local_bot_thread;

		network::server_ptr nserver;
		network::client_ptr nclient;
		network::client_ptr nbotclient;
		if(local_server) {
			nserver = std::make_shared<network::internal::server>();
			nclient = std::make_shared<network::internal::client>();
			nserver->add_peer(nclient);
			nclient->add_peer(nserver);
			
			nbotclient = std::make_shared<network::internal::client>();
			nserver->add_peer(nbotclient);
			nbotclient->add_peer(nserver);	

			local_server_thread.reset(new std::thread(local_server_code, gs, nserver));
			local_bot_thread.reset(new std::thread(ai::local_bot_code, b1, gs, nbotclient));
		} else {
			//nclient = std::make_shared<network::enet::client>(server_name, server_port);
		}
		e.set_netclient(nclient);
		e.set_active_player(p1);

 		e.add_process(std::make_shared<process::input>());
		e.add_process(std::make_shared<process::render>());
		e.add_process(std::make_shared<process::gui>());
		e.add_process(std::make_shared<process::ai>());
		e.add_process(std::make_shared<process::action>());
		// N.B. entity/map collision needs to come before entity/entity collisio;n
		e.add_process(std::make_shared<process::em_collision>());
		e.add_process(std::make_shared<process::ee_collision>());

		auto bw = gui::initiative::create(rectf(0.0f, 0.0f, 0.4f, 0.1f), gui::Justify::H_CENTER | gui::Justify::BOTTOM);
		e.add_widget(bw);

		auto info_win = gui::dialog::create(rectf(0.0f, 0.0f, 0.2f, 1.0f), gui::Justify::RIGHT | gui::Justify::TOP);
		e.add_widget(info_win);

		auto status_bar = gui::dialog::create(rectf(0.0f, 0.0f, 0.8f, 0.05f), gui::Justify::LEFT | gui::Justify::TOP);
		e.add_widget(status_bar);

		SDL_SetRenderDrawColor(wm.get_renderer(), 0, 0, 0, 255);
		while(running) {
			Uint32 cycle_start_tick = SDL_GetTicks();
			profile::timer tm;

			if(nclient) {
				nclient->process();
				game::Update* up;
				while((up = nclient->read_recv_queue()) != nullptr) {
					std::cerr << "client: Got message: " << up->id() << "\n";
					gs.apply(up);
					e.process_update(up);
					delete up;
				}
			}

			SDL_RenderClear(wm.get_renderer());
			try {
				running = e.update(60.0/1000.0);
			} catch(std::bad_weak_ptr& e) {
				ASSERT_LOG(false, "Bad weak ptr: " << e.what());
			}

			draw_perf_stats(e, tm.get_time());

			SDL_RenderPresent(wm.get_renderer());
	
			Uint32 delay = SDL_GetTicks() - cycle_start_tick;

			if(delay > FRAME_RATE) {
				//std::cerr << "CYCLE TOO LONG: " << delay << std::endl;
			} else {
				SDL_Delay(FRAME_RATE - delay);
			}
		}

		// This is mostly a local server thing to kill the server and bot threads
		// Basically we construct a message saying quit, then the server
		// sends that to the clients.
		if(nserver) {
			game::Update* up = new game::Update();
			up->set_id(-1);
			up->set_quit(true);
			nserver->write_recv_queue(up);
		}
		if(local_bot_thread && local_bot_thread->joinable()) {
			local_bot_thread->join();
		}
		if(local_server_thread && local_server_thread->joinable()) {
			local_server_thread->join();
		}
	} catch(std::exception& ex) {
		std::cerr << ex.what();
	}
	return 0;
}
