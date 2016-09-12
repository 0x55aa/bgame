#include "main_menu.hpp"
#include <rltk.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>
#include "menu_helper.hpp"
#include "constants.hpp"
#include "game_globals.hpp"

constexpr int BACKDROP_LAYER=1;
constexpr int LOG_LAYER=2;

using namespace rltk;
using namespace rltk::colors;

std::string main_menu::get_descriptive_noun() {
	random_number_generator rng;
	switch (rng.roll_dice(1,17)) {
		case 1 : return std::string("Stupidity");
		case 2 : return std::string("Idiocy");
		case 3 : return std::string("Dullness");
		case 4 : return std::string("Foolishness");
		case 5 : return std::string("Futility");
		case 6 : return std::string("Naievity");
		case 7 : return std::string("Senselessness");
		case 8 : return std::string("Shortsightedness");
		case 9 : return std::string("Triviality");
		case 10 : return std::string("Brainlessness");
		case 11 : return std::string("Inanity");
		case 12 : return std::string("Insensitivity");
		case 13 : return std::string("Indiscretion");
		case 14 : return std::string("Mindlessness");
		case 15 : return std::string("Moronism");
		case 16 : return std::string("Obtuseness");
		case 17 : return std::string("Unthinkingness");
	}
	return "Bugginess";
}

void main_menu::init() {
	auto window_size = get_window()->getSize();
	const int window_width = window_size.x;
	const int window_height = window_size.y;

	gui->add_owner_layer(BACKDROP_LAYER, 0, 0, window_width, window_height, resize_fullscreen, draw_splash_backdrop);
	gui->add_layer(LOG_LAYER, 0, 0, window_width, window_height, game_config.gui_font, resize_fullscreen, false);

	if (boost::filesystem::exists("world/savegame.dat")) {
		world_exists = true;
		selected = 0;
	}

	random_number_generator rng;
	switch (rng.roll_dice(1,6)) {
		case 1 : tagline = "Histories "; break;
		case 2 : tagline = "Chronicles "; break;
		case 3 : tagline = "Sagas "; break;
		case 4 : tagline = "Annals "; break;
		case 5 : tagline = "Narratives "; break;
		case 6 : tagline = "Recitals "; break;
	}
	tagline += "of " + get_descriptive_noun() + " and " + get_descriptive_noun();
}

void main_menu::destroy() {
	gui->delete_layer(BACKDROP_LAYER);
	gui->delete_layer(LOG_LAYER);
	clicked = false;
}

void main_menu::tick(const double duration_ms) {
	if (key_delay < 1.0 && is_window_focused()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			--selected;
			if (selected < 0) selected = 2;
			if (!world_exists && selected == 0) selected = 2;
			key_delay = 5.0;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			++selected;
			if (selected > 2) selected = 0;
			if (!world_exists && selected == 0) selected = 1;
			key_delay = 5.0;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
			clicked = true;
		}
	} else {
		key_delay -= duration_ms;
	}

	const int y_height = term(LOG_LAYER)->term_height;
	const int y_center = y_height / 2;

	int mouse_x, mouse_y;
	int font_w, font_h;
	std::tie(mouse_x, mouse_y) = get_mouse_position();
	std::tie(font_w, font_h) = term(LOG_LAYER)->get_font_size();
	const int terminal_x = mouse_x / font_w;
	const int terminal_y = mouse_y / font_h;
	const int term_width = term(LOG_LAYER)->term_width;
	const int half_term_width = term_width/2;

	if (terminal_y == y_center-3 && terminal_x > half_term_width-6 && terminal_x < half_term_width+7 && world_exists) {
		selected=0;
		if (get_mouse_button_state(rltk::button::LEFT)) clicked = true;
	}
	if (terminal_y == y_center-2 && terminal_x > half_term_width-6 && terminal_x < half_term_width+7) {
		selected=1;
		if (get_mouse_button_state(rltk::button::LEFT)) clicked = true;
	}
	if (terminal_y == y_center-1 && terminal_x > half_term_width-6 && terminal_x < half_term_width+7 && world_exists) {
		selected=2;
		if (get_mouse_button_state(rltk::button::LEFT)) clicked = true;
	}


	term(LOG_LAYER)->clear();
	term(LOG_LAYER)->print_center(y_center - 6, VERSION, WHITE, BLACK);
	term(LOG_LAYER)->print_center(y_center - 5, tagline, LIGHT_RED, BLACK);
	term(LOG_LAYER)->print_center(y_center + 5, "Powered by RLTK - the RogueLike Tool Kit", RED, BLACK);

	if (selected != 0) {
		if (world_exists) {
			term(LOG_LAYER)->print_center(y_center - 3, "Play the Game", WHITE, BLACK);
		} else {
			term(LOG_LAYER)->print_center(y_center - 3, "Play the Game", DARK_GREY, BLACK);			
		}
	} else {
		term(LOG_LAYER)->print_center(y_center - 3, "Play the Game", GREEN, BLACK);
	}

	if (selected != 1) {
		term(LOG_LAYER)->print_center(y_center - 2, "Generate The World", WHITE, BLACK);
	} else {
		term(LOG_LAYER)->print_center(y_center - 2, "Generate The World", GREEN, BLACK);
	}

	if (selected != 2) {
		term(LOG_LAYER)->print_center(y_center - 1, "Quit", WHITE, BLACK);
	} else {
		term(LOG_LAYER)->print_center(y_center - 1, "Quit", GREEN, BLACK);
	}
}
