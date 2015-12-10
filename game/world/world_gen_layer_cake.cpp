#include "world_gen_layer_cake.hpp"
#include "planet.hpp"
#include <iostream>

void make_world_layers ( heightmap_t* base_map, engine::random_number_generator &rng )
{
    // Altitude banding
    const int lowest_ground = min_heightmap_height( base_map );
    const int highest_ground = max_heightmap_height( base_map );
    const int n_bands = 20;
    const int range = highest_ground - lowest_ground;
    const int step = range / n_bands;
    
    std::cout << "Lowest: " << lowest_ground << ", highest: " << highest_ground << ", range: " << range << ", step: " << step << "\n";
    
    std::vector<uint8_t> bands ( NUMBER_OF_TILES_IN_THE_WORLD );
    for (int i=0; i<NUMBER_OF_TILES_IN_THE_WORLD; ++i) {
	const int altitude_at_point = base_map->operator[] ( i );
	bands[i] = ((altitude_at_point - lowest_ground) / step) + 128;
    }
    
    std::unique_ptr < planet_t > planet = std::make_unique < planet_t > ();
    
    // Make regions and cells
    for (int wy=0; wy<WORLD_HEIGHT; ++wy) {
	for (int wx=0; wx<WORLD_WIDTH; ++wx) {
	    const uint8_t region_index = planet->planet_idx ( wx, wy );
	    region_t * region_ptr = planet->create_region ( region_index );
	    
	    for (uint8_t y=0; y<REGION_HEIGHT; ++y) {
		for (uint8_t x=0; x<REGION_WIDTH; ++x) {
		    const int hidx = height_map_idx( (WORLD_WIDTH * wx) + x, (WORLD_HEIGHT * wy) + y );
		  
		    // Build upwards. Bottom level has to be solid. Then lava for min 3, biased by altitude. Then rock. Then water, if > 10.
		    tile_t * bottom = planet->get_tile( location_t{ region_index, x, y, 0 } );
		    bottom->solid = true;
		    bottom->base_tile_type = tile_type::FLAT;
		    bottom->ground = tile_ground::IGNEOUS;
		    bottom->covering = tile_covering::BARE;
		    bottom->climate = tile_climate::VERY_HOT;
		    
		    // Add magma
		    const int n_magma = 3 + bands [ hidx ] - 128;
		    uint8_t z = 1;
		    while ( z < n_magma+1) {
			tile_t * target = planet->get_tile( location_t{region_index, x, y, z} );
			  target->solid = true;
			  target->base_tile_type = tile_type::MAGMA;
			  target->covering = tile_covering::BARE;
			  target->climate = tile_climate::MOLTEN;
			++z;
		    }
		    
		    bottom = planet->get_tile( location_t{region_index, x, y, z} );
		    bottom->solid = true;
		    bottom->base_tile_type = tile_type::FLAT;
		    bottom->ground = tile_ground::IGNEOUS;
		    bottom->covering = tile_covering::BARE;
		    bottom->climate = tile_climate::VERY_HOT;
		    ++z;
		    
		    // Add solid rock
		    const int n_layers = rng.roll_dice(1, 6);
		    while ( z < bands[ hidx ]-n_layers ) {
			tile_t * target = planet->get_tile( location_t{region_index, x, y, z} );
			target->solid = true;
			target->base_tile_type = tile_type::FLAT;
			target->ground = tile_ground::IGNEOUS;
			target->covering = tile_covering::BARE;
			target->climate = tile_climate::SUBTERRANEAN;
			++z;
		    }
		    
		    while ( z < bands[ hidx ]-1 ) {
			tile_t * target = planet->get_tile( location_t{region_index, x, y, z} );
			target->solid = true;
			target->base_tile_type = tile_type::FLAT;
			target->ground = tile_ground::SEDIMENTARY;
			target->covering = tile_covering::BARE;
			target->climate = tile_climate::SUBTERRANEAN;
			++z;
		    }
		    
		    // The surface
		    tile_t * target = planet->get_tile( location_t{region_index, x, y, z} );
		    target->solid = true;
		    target->base_tile_type = tile_type::FLAT;
		    target->ground = tile_ground::SEDIMENTARY;
		    target->covering = tile_covering::GRASS;
		    target->climate = tile_climate::TEMPERATE;
		    
		    // Water decisions go here
		    
		}
	    }
	    
	    planet->save_region ( region_index );
	    planet->free_region ( region_index );
	}
    }

}