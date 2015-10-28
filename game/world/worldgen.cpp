#include "worldgen.h"

#include "world.h"
#include "geometry.h"
#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <tuple>
#include <cmath>
#include "../../engine/rng.h"
#include <iostream>
#include "../../engine/ecs/ecs.h"
#include "../../engine/ecs/entity_factory.h"
#include "../../engine/colors.h"
#include "../../engine/raws/raws.h"
#include "../../engine/globals.h"

using std::vector;
using std::map;
using std::make_unique;
using namespace engine;
using namespace engine::ecs;

namespace worldgen {

constexpr int worldgen_width = world::world_width*landblock_width;
constexpr int worldgen_height = world::world_height*landblock_height;
constexpr int worldgen_size = worldgen_height * worldgen_width;
typedef vector<short> height_map_t;

// Forward Declarations
void update_gui_heightmap ( height_map_t &altitude_map );

/* Index of the world grid. Constexpr shouldn't help here, but for some reason in
 * measured performance, it did. */
constexpr int idx ( const int x, const int y )
{
    return ( y * worldgen_width ) + x;
}

/*
 * Creates a height-map vector, sized to hold the world, and initialized with 1.
 */
height_map_t make_altitude_map()
{
    height_map_t altitude_map;
    altitude_map.resize ( worldgen_size );
    std::fill ( altitude_map.begin(), altitude_map.end(), 1 );
    return altitude_map;
}

/*
 * Creates a temperature-map vector, sized to hold the world, and initialized with a
 * y-based gradient
 */
height_map_t make_temperature_map( height_map_t &altitude_map )
{
    height_map_t temperature_map;
    temperature_map.resize ( worldgen_size );
    std::fill ( temperature_map.begin(), temperature_map.end(), 0 );
    constexpr double normal_temperature = 57.0;
    constexpr double total_variance = 120.0;
    constexpr double variance_half = 60.0;
    const double worldgen_h_double = worldgen_height;
    for (int y=0; y<worldgen_height; ++y) {
      const double y_double = y;
      const double y_extent = y_double/worldgen_h_double;
      for (int x=0; x<worldgen_width; ++x) {
	const int random_factor = engine::roll_dice(1,40) - 20;
	const double y_variance = (y_extent * total_variance) - variance_half;
	temperature_map[idx(x,y)] = normal_temperature + random_factor + y_variance;
      }
    }
    return temperature_map;
}

/*
 * Performs a smoothing run on altitude_map. Each cell becomes
 * the average of its neighbors height. To avoid bias, the
 * smoothing copies into a new vector, which is then copied
 * back.
 */
void smooth_altitude_map ( height_map_t &altitude_map )
{
    height_map_t new_map;
    new_map.resize ( worldgen_size );

    for ( int y=1; y<worldgen_height-1; ++y ) {
        for ( int x=1; x<worldgen_width-1; ++x ) {
            const int sum = altitude_map[idx ( x-1, y-1 )] +
                            altitude_map[idx ( x, y-1 )] +
                            altitude_map[idx ( x+1, y-1 )] +
                            altitude_map[idx ( x-1, y )] +
                            altitude_map[idx ( x, y )] +
                            altitude_map[idx ( x+1, y )] +
                            altitude_map[idx ( x-1, y+1 )] +
                            altitude_map[idx ( x, y+1 )] +
                            altitude_map[idx ( x+1, y+1 )];
            const short average = sum / 9;
            new_map[idx ( x,y )] = average;
        }
    }

    altitude_map.clear();
    std::copy ( new_map.begin(), new_map.end(), std::back_inserter ( altitude_map ) );
}

/*
 * Simulates a fault-line (more realistic than just adding blobs). Given a
 * starting and ending point, it iterates a line between the two and adds
 * (adjustment) to the point's height.
 */
void apply_fault_line_to_altitude_map ( height_map_t &altitude_map, const int start_x,
                                        const int start_y, const int end_x, const int end_y,
                                        const int adjustment )
{
    geometry::line_func ( start_x, start_y, end_x, end_y, [&altitude_map,adjustment] ( int tx, int ty ) {
        const int index = idx ( tx,ty );
        const int old_altitude = altitude_map[index];
        if ( ( old_altitude + adjustment ) < 32000 and ( old_altitude + adjustment ) > -32000 )
            altitude_map[index] += adjustment;
    } );
}

/*
 * Iterates the whole height map, and returns the minimum and maximum heights. A
 * frequency map is populated, containing how frequently each height occurs.
 * Returned as a tuple.
 */
std::tuple<short,short,map<short,int>> min_max_heights ( const height_map_t & altitude_map )
{
    short min_height = 32767;
    short max_height = -32768;
    map<short,int> frequency_map;
    for ( const short &h : altitude_map ) {
        if ( h < min_height ) min_height = h;
        if ( h > max_height ) max_height = h;

        auto finder = frequency_map.find ( h );
        if ( finder == frequency_map.end() ) {
            frequency_map[h] = 1;
        } else {
            int n = finder->second;
            ++n;
            frequency_map[h] = n;
        }
    }

    return std::make_tuple ( min_height, max_height, frequency_map );
}

/*
 * Used to store the height levels at which each type of terrain
 * occurs.
 */
struct altitude_map_levels {
    short water_level;
    short plains_level;
    short hills_level;
    short mountains_level;
};

/*
 * Utility function; iterates through a frequency map until it has reached a level in which
 * the sum of previous levels equals (or exceeds) target. This is used to specify, e.g. "1/3
 * of the map should be water" - send in worldgen_size/3 as the target, and it will find
 * the altitude that satisfies this.
 */
short find_layer_level ( const map<short,int> &levels, const int &target, const short &min_layer )
{
    int total = 0;
    short level = min_layer;
    while ( total < target ) {
        auto finder = levels.find ( level );
        if ( finder != levels.end() ) {
            total += finder->second;
        }
        ++level;
    }
    return level;
}

/*
 * Determine the altitudes at which water, plains, hills and mountains are found.
 */
altitude_map_levels determine_levels ( const std::tuple<short,short,map<short,int>> &min_max_freq )
{
    altitude_map_levels result;

    // The goal is to have 1/3rd of the map be water.
    const int one_third_total = worldgen_size / 3;
    result.water_level = find_layer_level ( std::get<2> ( min_max_freq ), one_third_total, std::get<0> ( min_max_freq ) );

    // The second third should be flat terrain
    const int two_third_total = one_third_total*2;
    result.plains_level = find_layer_level ( std::get<2> ( min_max_freq ), two_third_total, std::get<0> ( min_max_freq ) );

    // The next third is half hills, half mountains
    const int one_sixth_total = worldgen_size / 6;
    const int hills_target = two_third_total + one_sixth_total;
    result.hills_level = find_layer_level ( std::get<2> ( min_max_freq ), hills_target, std::get<0> ( min_max_freq ) );
    result.mountains_level = std::get<1> ( min_max_freq ) +1;

    return result;
}

/*
 * Convert the basic height map into tile types on a simple altitude basis.
 * Performed per landblock.
 */
void create_base_tile_types ( const int region_x, const int region_y, land_block &region,
                              const height_map_t &altitude_map, const altitude_map_levels &levels,
			      const height_map_t &temperature_map
			    )
{
    for ( int y=0; y<landblock_height; ++y ) {
        for ( int x=0; x<landblock_width; ++x ) {
            std::lock_guard<std::mutex> lock ( worldgen_mutex );
            const int amp_x = region_x + x;
            const int amp_y = region_y + y;
            const short altitude = altitude_map[idx ( amp_x, amp_y )];
            const int tile_idx = region.idx ( x,y );
            if ( altitude < levels.water_level ) {
                region.tiles[tile_idx].base_tile_type = tile_type::WATER;
		region.tiles[tile_idx].altitude = 1;
            } else {
                region.tiles[tile_idx].base_tile_type = tile_type::FLAT;
		region.tiles[tile_idx].altitude = altitude;	    
            }
	    region.tiles[tile_idx].temperature = temperature_map[idx ( amp_x, amp_y )];
	    if (x==0 or x==(landblock_width-1)) {
	      region.tiles[tile_idx].surface_normal = 0;
	    } else {
	      const int left_altitude = region.tiles[tile_idx-1].altitude;
	      const int right_altitude = altitude_map[idx ( amp_x+1, amp_y )];
	      const int difference = left_altitude - right_altitude;
	      // If the opposite is the altitude difference, and the adjacent is 10
	      // Tan(angle) = Opposite * Adjacent
	      if (difference > 0) {
		region.tiles[tile_idx].surface_normal = std::atan(difference) * (180.0 / M_PI);
	      } else {
		region.tiles[tile_idx].surface_normal = 90.0 + (90-(std::atan(0-difference) * (180.0 / M_PI)));
	      }
	      //std::cout << left_altitude << "-" << right_altitude << " --> " << region.tiles[tile_idx].surface_normal << "\n";
	    }
        }
    }
}

/*
 * Performs a simplified marching-squares over a heightmap region, creating up ramp squares
 * as it finds contour lines.
 */
void walk_contours ( const int region_x, const int region_y, land_block &region,
                     const height_map_t &altitude_map, const altitude_map_levels &levels)
{
    const short min_height = levels.water_level;
    const short max_height = levels.mountains_level;
    const short contour_distance = (max_height - min_height)/10;

    // Threshold: true for above contour, false for below
    vector<bool> threshold(tiles_per_landblock);

    unsigned char level_band = 0;
    for (int contour=min_height; contour<max_height; contour += contour_distance) {
        // Populate the thresholds
        for ( int y=0; y<landblock_height; ++y ) {
            for ( int x=0; x<landblock_width; ++x ) {
                const int amp_x = region_x + x;
                const int amp_y = region_y + y;
                const short altitude = altitude_map[idx ( amp_x, amp_y )];
                const int tile_idx = region.idx ( x,y );

                if (altitude < contour) {
                    threshold[tile_idx] = 0;
                } else {
                    threshold[tile_idx] = 1;
		    region.tiles[tile_idx].level_band = level_band;
                }
            }
        }
        ++level_band;

        // Walk the threshold map
        for (int contour=min_height; contour<max_height; contour += contour_distance) {
            // Populate the thresholds
            for ( int y=0; y<landblock_height-1; ++y ) {
                for ( int x=0; x<landblock_width-1; ++x ) {
                    const int tile_idx = region.idx ( x,y );

                    int TL = region.idx ( x, y );
                    int TR = region.idx ( x+1, y );
                    int BR = region.idx ( x+1, y+1 );
                    int BL = region.idx ( x, y+1 );

                    unsigned char value = 0;
                    if (threshold[TL]) value += 1;
                    if (threshold[TR]) value += 2;
                    if (threshold[BR]) value += 4;
                    if (threshold[BL]) value += 8;

                    if (value != 0 && value != 15) {
                        std::lock_guard<std::mutex> lock ( worldgen_mutex );
                        region.tiles[tile_idx].base_tile_type = tile_type::RAMP;
                    }
                }
            }
        }
    }
}

/*
 * Traverses the edge of the water, and adds beach tiles to the transition area
 */
void walk_waters_edge ( const int region_x, const int region_y, land_block &region,
                     const height_map_t &altitude_map, const altitude_map_levels &levels)
{
    const short min_height = levels.water_level;
    const short max_height = levels.mountains_level;
    const short contour_distance = (max_height - min_height)/10;

    // Threshold: true for above contour, false for below
    vector<bool> threshold(tiles_per_landblock);

    // Populate the thresholds
    for ( int y=0; y<landblock_height; ++y ) {
	for ( int x=0; x<landblock_width; ++x ) {
	    const int tile_idx = region.idx ( x,y );

	    if (region.tiles[tile_idx].base_tile_type == tile_type::WATER) {
		threshold[tile_idx] = 0;
	    } else {
		threshold[tile_idx] = 1;
	    }
	}
    }

    // Walk the threshold map
    for (int contour=min_height; contour<max_height; contour += contour_distance) {
	// Populate the thresholds
	for ( int y=0; y<landblock_height-1; ++y ) {
	    for ( int x=0; x<landblock_width-1; ++x ) {
		const int tile_idx = region.idx ( x,y );

		int TL = region.idx ( x, y );
		int TR = region.idx ( x+1, y );
		int BR = region.idx ( x+1, y+1 );
		int BL = region.idx ( x, y+1 );

		unsigned char value = 0;
		if (threshold[TL]) value += 1;
		if (threshold[TR]) value += 2;
		if (threshold[BR]) value += 4;
		if (threshold[BL]) value += 8;

		if (value != 0 && value != 15) {
		    std::lock_guard<std::mutex> lock ( worldgen_mutex );
		    region.tiles[tile_idx].base_tile_type = tile_type::BEACH;
		}
	    }
	}
    }
}

void adjust_temperature_by_altitude(land_block &region) {
  for (tile &t : region.tiles) {
    t.temperature -= (t.level_band-3);
  }
}

void tile_calc(land_block &region) {
  for (tile &t : region.tiles) {
    t.worldgen_tile_calc();
  }
}

/*
 * Converts the entire world altitude map into landblocks, and
 * saves them to disk.
 */
void convert_altitudes_to_tiles ( height_map_t &altitude_map, height_map_t &temperature_map )
{
    const std::tuple<short,short,map<short,int>> min_max_freq = min_max_heights ( altitude_map );
    const altitude_map_levels levels = determine_levels ( min_max_freq );

    {
        std::lock_guard<std::mutex> lock ( worldgen_mutex );
        current_landblock = nullptr;
    }

    for ( int wy=0; wy<world::world_height; ++wy ) {
        for ( int wx=0; wx<world::world_width; ++wx ) {
            // Create the world tile object
            land_block region;
            {
                std::lock_guard<std::mutex> lock ( worldgen_mutex );
                for ( tile &t : region.tiles ) t.base_tile_type = tile_type::FLAT;
                current_landblock = &region;
            }
            region.index = world::world_idx ( wx,wy );
            const int region_x = wx*landblock_width;
            const int region_y = wy*landblock_height;

            create_base_tile_types ( region_x, region_y, region, altitude_map, levels, temperature_map );
            walk_contours( region_x, region_y, region, altitude_map, levels );
            walk_waters_edge( region_x, region_y, region, altitude_map, levels );	    
	    adjust_temperature_by_altitude(region);
	    tile_calc(region);

            // Serialize it to disk
            {
                std::lock_guard<std::mutex> lock ( worldgen_mutex );
                region.save();
                current_landblock = nullptr;
            }
        }
    }
}

/*
 * Basic heightmap generation algorithm. Calls the previously
 * defined functions.
 */
void create_heightmap_world()
{
    height_map_t altitude_map = make_altitude_map();
    const int num_fault_lines = worldgen_size/10000;

    {
        std::lock_guard<std::mutex> lock ( worldgen_mutex );
        progress = FAULTLINES;
        heightmap_progress = new vector<short>();
        heightmap_progress->resize ( world::world_size );
    }
    update_gui_heightmap ( altitude_map ); // Handles its own locking

    for ( int i=0; i<num_fault_lines; ++i ) {
        int sx = engine::roll_dice ( 1, worldgen_width-1 );
        int sy = engine::roll_dice ( 1, worldgen_height-1 );
        int ex = engine::roll_dice ( 1, worldgen_width-1 );
        int ey = engine::roll_dice ( 1, worldgen_height-1 );

        apply_fault_line_to_altitude_map ( altitude_map,
                                           sx,
                                           sy,
                                           ex,
                                           ey,
                                           1000
                                         );
        if ( i % 300 == 0 ) update_gui_heightmap ( altitude_map );
    }
    for ( int i=0; i<64; ++i ) {
        smooth_altitude_map ( altitude_map );
        if ( i % 16 == 0 ) update_gui_heightmap ( altitude_map );
    }
    {
        std::lock_guard<std::mutex> lock ( worldgen_mutex );
        progress = MAKETILES;
    }
    
    // Make a rainfall map
    // Make a temperature map
    height_map_t temperature_map = make_temperature_map( altitude_map );
    smooth_altitude_map( temperature_map );
    
    convert_altitudes_to_tiles ( altitude_map, temperature_map );
    {
        std::lock_guard<std::mutex> lock ( worldgen_mutex );
        progress = DONE;
    }
}

std::pair<int,int> get_starting_location() {
    // TODO: Something that actually takes into account the location!
    return std::make_pair(landblock_width/2, landblock_height/2);
}

void crash_trail(const std::pair<int,int> &starting_location) {
    for (int x=0; x<starting_location.first+4; ++x) {
	for (int y=(starting_location.second-3); y<(starting_location.second)+4; ++y) {
	    const int tile_idx = world::current_region->idx(x,y);
	    if (world::current_region->tiles[tile_idx].base_tile_type != tile_type::WATER) {
		world::current_region->tiles[tile_idx].covering = tile_covering::BARE;
		// TODO: Chance of being on fire!
	    }
	}
	// TODO: Ramp on either side?
    }
    world::current_region->save();
}

void add_cordex(const int &x, const int &y) {
    entity cordex;
    engine::globals::ecs->add_entity(cordex);
    world::cordex_handle = cordex.handle;
    engine::globals::ecs->add_component(cordex, debug_name_component("Cordex"));
    engine::globals::ecs->add_component(cordex, position_component(x,y));
    engine::globals::ecs->add_component(cordex, viewshed_component(penetrating,16));
    engine::globals::ecs->add_component(cordex, calendar_component(0L));
    engine::globals::ecs->add_component(cordex, renderable_component(15, cyan, black));
    engine::globals::ecs->add_component(cordex, obstruction_component());
}

void add_solar_collector(const int x, const int y) {
    engine::raws::create_structure_from_raws("Solar Collector", x, y);
}

void add_cordex_console(const int x, const int y, const unsigned char symbol) {
    entity console;
    engine::globals::ecs->add_entity(console);
    engine::globals::ecs->add_component(console, debug_name_component("Cordex Console"));
    engine::globals::ecs->add_component(console, position_component(x,y));
    engine::globals::ecs->add_component(console, renderable_component(symbol, dark_cyan, black));
    engine::globals::ecs->add_component(console, obstruction_component());
}

void add_food_replicator(const int x, const int y) {
    engine::raws::create_structure_from_raws("Small Replicator", x, y);
}

void add_storage_unit(const int x, const int y) {
    engine::raws::create_structure_from_raws("Storage Unit", x, y);
}

void add_structural_element(const int x, const int y, unsigned char glyph, bool block=true) {
  switch (glyph) {
    case 16 : engine::raws::create_structure_from_raws("Ship Front", x, y); break;
    case 186 : engine::raws::create_structure_from_raws("Ship Wall NS", x, y); break;
    case 200 : engine::raws::create_structure_from_raws("Ship Wall NE", x, y); break;
    case 201 : engine::raws::create_structure_from_raws("Ship Wall SE", x, y); break;
    case 219 : engine::raws::create_structure_from_raws("Ship Superstructure", x, y); break;
    case 46 : engine::raws::create_structure_from_raws("Ship Floor", x, y); break;
    case 205 : engine::raws::create_structure_from_raws("Ship Wall EW", x, y); break;
    default : std::cout << "Oops - missed a structure, code " << +glyph << "\n";
  }
  /*
    entity structure;
    structure.handle = next_entity_handle();
    add_entity(structure);
    add_component(structure, position_component(x,y));
    add_component(structure, renderable_component(glyph, white, black));
    if (block) add_component(structure, obstruction_component());*/
}

void add_ship_hull(const std::pair<int,int> &starting_location) {
    const int x = starting_location.first;
    const int y = starting_location.second;
    
    // Cordex at (x,y)
    add_cordex(x,y);
    
    // Solar collector at (x-1,y-1), (x+1,y-1), (x-1,y+1), (x+1,y+1)
    add_solar_collector(x-1, y-1);
    add_solar_collector(x+1, y-1);
    add_solar_collector(x-1, y+1);
    add_solar_collector(x+1, y+1);
    
    // Console constructions at (x-1,y), (x+1,y), (x,y-1), (x,y+1)
    engine::raws::create_structure_from_raws("Education Console", x-1, y);
    engine::raws::create_structure_from_raws("Scanner Console", x+1, y);
    engine::raws::create_structure_from_raws("Defense Console", x, y-1);
    engine::raws::create_structure_from_raws("Communications Console", x, y+1);
    
    // Refridgerator/Food Replicator at (x+4,y)
    add_food_replicator(x+4, y);
    
    // Storage unit at (x+4,y-1) and (x+4,y+1)
    add_storage_unit(x+4, y-1);
    add_storage_unit(x+4, y+1);
    
    // Escape pod structure
    add_structural_element(x-4, y-3, 201); // TL Corner
    add_structural_element(x-4, y+3, 200); // BL Corner
    for (int i=0; i<6; ++i) {
	add_structural_element(x-3+i, y-3, 205); // Upper hull wall
	add_structural_element(x-3+i, y+3, 205); // Lower hull wall
    }
    for (int i=0; i<5; ++i) {
      if (i != 2) {
	add_structural_element(x-4, y-2+i, 186); // Add left wall
      } else {
	// TODO: Door goes here!
      }
    }
    
    add_structural_element(x+3, y-3, 16); // Front
    add_structural_element(x+4, y-2, 16); // Front
    add_structural_element(x+5, y-1, 16); // Front
    add_structural_element(x+6, y, 16); // Front
    add_structural_element(x+5, y+1, 16); // Front
    add_structural_element(x+4, y+2, 16); // Front
    add_structural_element(x+3, y+3, 16); // Front
    
    for (int i=0; i<5; ++i) {
      add_structural_element(x-3, y-2+i, '.', false);
      add_structural_element(x-2, y-2+i, '.', false);
      add_structural_element(x+2, y-2+i, '.', false);
      add_structural_element(x+3, y-2+i, '.', false);
    }
    add_structural_element(x-1, y-2, '.', false);
    add_structural_element(x, y-2, '.', false);
    add_structural_element(x+1, y-2, '.', false);
    add_structural_element(x-1, y+2, '.', false);
    add_structural_element(x, y+2, '.', false);
    add_structural_element(x+1, y+2, '.', false);
    add_structural_element(x+5, y, 219);
/*
-------|
|@ @ @  \
|  ###  S\
%  #*#  F |
|  ###  S/
|@ @ @  /
-------|
*/
}

void setup_initial_game() {
    // Get the engine ready
    //engine::ecs::init();
    int widx = world::world_idx(16,16);
    world::current_region = new land_block(widx);
    
    // Locate a spot for the crash
    std::pair<int,int> starting_location = get_starting_location();
    
    // Add bare/scorched ground in a line up to the crash so it looks like a hard landing
    crash_trail(starting_location);
    
    // Add the ship superstructure, doors, power, sensors, cordex
    add_ship_hull(starting_location);
    
    // Create some settlers and put them in the ship
    entity settler = make_test_entity(starting_location.first, starting_location.second-2);
    engine::globals::ecs->add_entity(settler);
    
    // Add the camera
    engine::ecs::entity camera = engine::ecs::make_camera_entity();
    engine::globals::ecs->add_entity(camera);
    
    // Persist and quit
    world::stored_power = 25;
    engine::globals::ecs->save_game();
    engine::globals::ecs->done();
}

/********* GUI Releated Functionality */

/*
 * External interface to world creation. Eventually, this needs to
 * be extended to provide some feedback fo the user.
 */
void build_world()
{
    create_heightmap_world();
    // TODO: Walk the map creating boundary regions (e.g. beaches, cliffs)
    // TODO: For the far future, biomes, history and goodies.

    // TODO: Find a place to crash the ship to let us start having something playable
    setup_initial_game();
}

// For communication with the GUI
std::mutex worldgen_mutex;
worldgen_progress progress = INIT;
vector<short> * heightmap_progress; // Used for sharing height-map progress with the GUI
land_block * current_landblock; // Also used for GUI communication

/*
 * Creates a summary map of a height map, for the world-gen gui.
 */
void update_gui_heightmap ( height_map_t &altitude_map )
{
    std::lock_guard<std::mutex> lock ( worldgen_mutex );

    // Determine minimum and maximum overall height
    short min_height = 32767;
    short max_height = -32768;
    for ( const short &h : altitude_map ) {
        if ( h < min_height ) min_height = h;
        if ( h > max_height ) max_height = h;
    }

    int range = ( max_height - min_height );
    if ( range == 0 ) range = 1;

    for ( int y=0; y<world::world_height; ++y ) {
        for ( int x=0; x<world::world_width; ++x ) {
            int accumulator = min_height;
            for ( int wy=0; wy<landblock_height; ++wy ) {
                for ( int wx=0; wx<landblock_width; ++wx ) {
                    const int tx = ( x * landblock_width ) + wx;
                    const int ty = ( y * landblock_height ) + wy;
                    const int index = idx ( tx,ty );
                    if ( altitude_map[index] > accumulator ) accumulator = altitude_map[index];
                }
            }
            const double average_height = accumulator;
            const double percent = ( average_height / range );
            const short col = 127 * percent;
            heightmap_progress->operator[] ( world::world_idx ( x,y ) ) = col;

        }
    }

}


}
