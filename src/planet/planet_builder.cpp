#include "planet_builder.hpp"
#include "../raws/raws.hpp"
#include "builder/settler_builder.hpp"
#include "builder/map_builder.hpp"
#include "builder/biome_builder.hpp"
#include "builder/region_builder.hpp"
#include "builder/history_builder.hpp"
#include "builder/river_builder.hpp"
#include "../FastNoise/FastNoise.h"

#include <atomic>
#include <iostream>
#include <limits>
#include <thread>
#include <rltk.hpp>
#include <sstream>

using namespace rltk;
using namespace rltk::colors;

std::atomic<bool> planet_build_done;
std::mutex planet_builder_lock;
std::unique_ptr<std::vector<worldgen_display_t>> planet_builder_display;
std::string planet_builder_status = "";

bool is_planet_build_complete() {
	return planet_build_done.load();
}

void setup_build_planet() {
	planet_builder_display = std::make_unique<std::vector<worldgen_display_t>>();
	planet_builder_display->resize(WORLD_TILES_COUNT);
	std::fill(planet_builder_display->begin(), planet_builder_display->end(), worldgen_display_t{});
}

inline void set_planet_display_char(const int &block_idx, const int &idx, planet_t &planet) {
    // Set the altitude component
    (*planet_builder_display.get())[idx].altitude = planet.landblocks[block_idx].height;
    (*planet_builder_display.get())[idx].background = rltk::colors::BLACK;

    const uint8_t zero = 0;
    const int biome_idx = planet.landblocks[block_idx].biome_idx;
    const rltk::color_t bg = rltk::colors::BLACK;
    uint8_t col = planet.landblocks[block_idx].height;

    if (planet.landblocks[block_idx].type == block_type::NONE) {
        (*planet_builder_display.get())[idx].terrain_glyph = 178;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, zero, col};
    } else if (planet.landblocks[block_idx].type == block_type::WATER) {
        (*planet_builder_display.get())[idx].terrain_glyph = 178;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, zero, col};
    } else if (planet.landblocks[block_idx].type == block_type::PLAINS) {
        (*planet_builder_display.get())[idx].terrain_glyph = 178;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else if (planet.landblocks[block_idx].type == block_type::HILLS) {
        (*planet_builder_display.get())[idx].terrain_glyph = 30;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else if (planet.landblocks[block_idx].type == block_type::MARSH) {
        (*planet_builder_display.get())[idx].terrain_glyph = '~';
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else if (planet.landblocks[block_idx].type == block_type::PLATEAU) {
        (*planet_builder_display.get())[idx].terrain_glyph = 239;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else if (planet.landblocks[block_idx].type == block_type::HIGHLANDS) {
        (*planet_builder_display.get())[idx].terrain_glyph = 178;
        (*planet_builder_display.get())[idx].foreground = color_t{col, col, col};
    } else if (planet.landblocks[block_idx].type == block_type::COASTAL) {
        (*planet_builder_display.get())[idx].terrain_glyph = 176;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else if (planet.landblocks[block_idx].type == block_type::SALT_MARSH) {
        (*planet_builder_display.get())[idx].terrain_glyph = 247;
        (*planet_builder_display.get())[idx].foreground = color_t{zero, col, zero};
    } else {
        (*planet_builder_display.get())[idx].terrain_glyph = 30;
        (*planet_builder_display.get())[idx].foreground = color_t{col, col, col};
    }

    for (const river_t &r : planet.rivers) {
        if (planet.idx(r.start_x, r.start_y) == block_idx) {
            (*planet_builder_display.get())[idx].terrain_glyph = '%';
            (*planet_builder_display.get())[idx].foreground = color_t{0, 0, 255};
        }
        for (const river_step_t &s : r.steps) {
            if (planet.idx(s.x, s.y) == block_idx) {
                (*planet_builder_display.get())[idx].terrain_glyph = '%';
                (*planet_builder_display.get())[idx].foreground = color_t{0, 0, 255};
            }
        }
    }

    if (!planet.civs.civs.empty()) {
        std::size_t owner_civ = planet.civs.region_info[block_idx].owner_civ;
        if (owner_civ != 0) {
            color_t civ_color = color_t{ planet.civs.civs[owner_civ].r, planet.civs.civs[owner_civ].g, planet.civs.civs[owner_civ].b };
            (*planet_builder_display.get())[idx].unit_glyph = planet.civs.civs[owner_civ].glyph;
        } else {
            (*planet_builder_display.get())[idx].unit_glyph = 0;
        }
    }
}

void planet_display_update_zoomed(planet_t &planet, const int world_x, const int world_y) {
	planet_builder_lock.lock();
    for (int y=0; y<WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            set_planet_display_char(planet.idx(x,y), planet.idx(x,y), planet);
        }
    }
	planet_builder_lock.unlock();
}

void planet_display_update_altitude(planet_t &planet) {
    planet_builder_lock.lock();
    for (int y=0; y<WORLD_HEIGHT; ++y) {
        for (int x = 0; x < WORLD_WIDTH; ++x) {
            set_planet_display_char(planet.idx(x,y), planet.idx(x,y), planet);
        }
    }
    planet_builder_lock.unlock();
}

void set_worldgen_status(const std::string &status) {
	planet_builder_lock.lock();
	planet_builder_status = status;
	planet_builder_lock.unlock();
}

void builder_save_planet(planet_t &planet) {
	planet_builder_lock.lock();
	planet_builder_status = "Saving the world. To disk, sadly.";
	planet_builder_lock.unlock();
	save_planet(planet);
	//std::this_thread::sleep_for(std::chrono::seconds(10));
}

void build_planet(const int &seed, const int &water_divisor, const int &plains_divisor, const int &starting_settlers,
    const bool &strict_beamdown)
{
	planet_build_done.store(false);

	random_number_generator rng(seed);
	planet_t planet;
	planet.rng_seed = seed;
	const int perlin_seed = rng.roll_dice(1, std::numeric_limits<int>::max());
	planet.perlin_seed = perlin_seed;
    planet.water_divisor = water_divisor;
    planet.plains_divisor = plains_divisor;
    planet.starting_settlers = starting_settlers;
    planet.strict_beamdown = strict_beamdown;
	//std::cout << "Planet seed: " << planet.perlin_seed << "\n";

	// Make a zero-height map
	planet_zero_fill(planet);

	// Noise-based world map
	FastNoise noise = planet_noise_map(planet, perlin_seed);

	// Divide types by height
	planet_base_type_allocation(planet);

	// Find coastlines
	planet_mark_coastlines(planet);

	// Adjust rainfall
	planet_rainfall(planet);

	// Make a biome map
	build_biomes(planet, rng);

	// Run rivers
	planet_rivers(planet, rng);

	// Run history
	planet_build_initial_civs(planet, rng);
	planet_build_initial_history(planet, rng);

	// Save it to disk
	builder_save_planet(planet);

	// Select a crash site
	auto crash_site = builder_select_starting_region(planet);
	planet_display_update_zoomed(planet, crash_site.first, crash_site.second);

	// Materialize this region
	build_region(planet, crash_site, rng, noise);
	save_planet(planet);

	planet_build_done.store(true);
}