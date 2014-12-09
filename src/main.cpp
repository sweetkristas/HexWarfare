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

#include "libwebsockets.h"

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

static unsigned int opts;
static int was_closed;
static int deny_deflate;
static int deny_mux;
static struct libwebsocket *wsi_mirror;
static int mirror_lifetime = 0;
static volatile int force_exit = 0;
static int longlived = 0;

enum demo_protocols {

	PROTOCOL_DUMB_INCREMENT,
	PROTOCOL_LWS_MIRROR,

	/* always last */
	DEMO_PROTOCOL_COUNT
};


/* dumb_increment protocol */

static int
callback_dumb_increment(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		fprintf(stderr, "callback_dumb_increment: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		fprintf(stderr, "LWS_CALLBACK_CLIENT_CONNECTION_ERROR\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLOSED:
		fprintf(stderr, "LWS_CALLBACK_CLOSED\n");
		was_closed = 1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		((char *)in)[len] = '\0';
		fprintf(stderr, "rx %d '%s'\n", (int)len, (char *)in);
		break;

	/* because we are protocols[0] ... */

	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		if ((strcmp((char*)in, "deflate-stream") == 0) && deny_deflate) {
			fprintf(stderr, "denied deflate-stream extension\n");
			return 1;
		}
		if ((strcmp((char*)in, "deflate-frame") == 0) && deny_deflate) {
			fprintf(stderr, "denied deflate-frame extension\n");
			return 1;
		}
		if ((strcmp((char*)in, "x-google-mux") == 0) && deny_mux) {
			fprintf(stderr, "denied x-google-mux extension\n");
			return 1;
		}

		break;

	default:
		break;
	}

	return 0;
}


static int
callback_lws_mirror(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
	//unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 4096 + LWS_SEND_BUFFER_POST_PADDING];
	int l = 0;
	//int n;

	switch (reason) {

	case LWS_CALLBACK_CLIENT_ESTABLISHED:

		fprintf(stderr, "callback_lws_mirror: LWS_CALLBACK_CLIENT_ESTABLISHED\n");

		mirror_lifetime = 10 + (random() & 1023);
		/* useful to test single connection stability */
		if (longlived)
			mirror_lifetime += 50000;

		fprintf(stderr, "opened mirror connection with "
				     "%d lifetime\n", mirror_lifetime);

		/*
		 * mirror_lifetime is decremented each send, when it reaches
		 * zero the connection is closed in the send callback.
		 * When the close callback comes, wsi_mirror is set to NULL
		 * so a new connection will be opened
		 */

		/*
		 * start the ball rolling,
		 * LWS_CALLBACK_CLIENT_WRITEABLE will come next service
		 */

		libwebsocket_callback_on_writable(context, wsi);
		break;

	case LWS_CALLBACK_CLOSED:
		fprintf(stderr, "mirror: LWS_CALLBACK_CLOSED mirror_lifetime=%d\n", mirror_lifetime);
		wsi_mirror = NULL;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
/*		fprintf(stderr, "rx %d '%s'\n", (int)len, (char *)in); */
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:

		/*for (n = 0; n < 1; n++)
			l += sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING + l],
					LWS_SEND_BUFFER_PRE_PADDING,
					"c #%06X %d %d %d;",
					(int)random() & 0xffffff,
					(int)random() % 500,
					(int)random() % 250,
					(int)random() % 24);

		n = libwebsocket_write(wsi,
		   &buf[LWS_SEND_BUFFER_PRE_PADDING], l, LWS_WRITE_TEXT);

		if (n < 0)
			return -1;
		if (n < l) {
			lwsl_err("Partial write LWS_CALLBACK_CLIENT_WRITEABLE\n");
			return -1;
		}*/

		mirror_lifetime--;
		if (!mirror_lifetime) {
			fprintf(stderr, "closing mirror session\n");
			return -1;
		} else
			/* get notified as soon as we can write again */
			libwebsocket_callback_on_writable(context, wsi);
		break;

	default:
		break;
	}

	return 0;
}

int lws_test()
{
	int n = 0;
	int port = 7681;
	int use_ssl = 0;
	const char *address = "localhost";
	struct libwebsocket *wsi_dumb;
	int ietf_version = -1; /* latest */

	static struct libwebsocket_protocols protocols[] = {
		{
			"dumb-increment-protocol,fake-nonexistant-protocol",
			callback_dumb_increment,
			0,
			20,
		},
		{
			"fake-nonexistant-protocol,lws-mirror-protocol",
			callback_lws_mirror,
			0,
			128,
		},
		{ NULL, NULL, 0, 0 } /* end */
	};

	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
#ifndef LWS_NO_EXTENSIONS
	info.extensions = libwebsocket_get_internal_extensions();
#endif
	info.gid = -1;
	info.uid = -1;

	struct libwebsocket_context *context = libwebsocket_create_context(&info);
	if (context == NULL) {
		fprintf(stderr, "Creating libwebsocket context failed\n");
		return 1;
	}

	wsi_dumb = libwebsocket_client_connect(context, address, port, use_ssl,
			"/", "", "",
			 protocols[PROTOCOL_DUMB_INCREMENT].name, ietf_version);

	if (wsi_dumb == NULL) {
		fprintf(stderr, "libwebsocket connect failed\n");
		goto bail;
	}

	fprintf(stderr, "Waiting for connect...\n");

	n = 0;
	while (n >= 0 && !was_closed && !force_exit) {
		n = libwebsocket_service(context, 10);

		if (n < 0)
			continue;

		if (wsi_mirror)
			continue;

		/* create a client websocket using mirror protocol */

		wsi_mirror = libwebsocket_client_connect(context,
			address, port, use_ssl,  "/",
			"", "",
			protocols[PROTOCOL_LWS_MIRROR].name, ietf_version);

		if (wsi_mirror == NULL) {
			fprintf(stderr, "libwebsocket "
					      "mirror connect failed\n");
			goto bail;
		}
	}

bail:
	fprintf(stderr, "Exiting\n");

	libwebsocket_context_destroy(context);

	return 1;
}

void create_gui(engine& eng)
{
	auto button_label = gui::label::create(rectf(), gui::Justify::H_CENTER | gui::Justify::V_CENTER, "End Turn", graphics::color(255,255,0), 16);
	rectf area(-0.02f,-0.02f,button_label->get_area().w()+0.05f,button_label->get_area().h()+0.02f);
	auto end_turn_button = gui::button::create(area, gui::Justify::BOTTOM | gui::Justify::RIGHT, std::bind(&engine::end_turn, &eng), button_label);
	eng.add_widget(end_turn_button);
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

		//lws_test();

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
