#include "region.hpp"
#include <rltk.hpp>
#include <string>
#include "../raws/raws.hpp"
//#include <Poco/InflatingStream.h>
//#include <Poco/DeflatingStream.h>

using namespace rltk;

void save_region(const region_t &region) {
	std::string region_filename = "world/region_" + std::to_string(region.region_x) + "_" + std::to_string(region.region_y) + ".dat";
	std::fstream deflate(region_filename, std::ios::out | std::ios::binary);
	//Poco::DeflatingOutputStream deflate(lbfile, Poco::DeflatingStreamBuf::STREAM_GZIP);
	serialize(deflate, region.region_x);
	serialize(deflate, region.region_y);
	serialize(deflate, region.biome_idx);
	serialize(deflate, region.tiles.size());
	for (const tile_t &tile : region.tiles) {
		serialize(deflate, tile.base_type);
		serialize(deflate, tile.contents);
		serialize(deflate, tile.liquid);
		serialize(deflate, tile.temperature);
		serialize(deflate, tile.flags);
	}
}

region_t load_region(const int region_x, const int region_y) {
	std::string region_filename = "world/region_" + std::to_string(region_x) + "_" + std::to_string(region_y) + ".dat";
	std::fstream inflate(region_filename, std::ios::in | std::ios::binary);
	//Poco::InflatingInputStream inflate(lbfile, Poco::InflatingStreamBuf::STREAM_GZIP);
	region_t region;

	deserialize(inflate, region.region_x);
	deserialize(inflate, region.region_y);
	deserialize(inflate, region.biome_idx);
	std::size_t number_of_tiles;
	deserialize(inflate, number_of_tiles);
	for (std::size_t i=0; i<number_of_tiles; ++i) {
		tile_t tile;
		deserialize(inflate, tile.base_type);
		deserialize(inflate, tile.contents);
		deserialize(inflate, tile.liquid);
		deserialize(inflate, tile.temperature);
		deserialize(inflate, tile.flags);
		region.tiles[i] = tile;
	}

	return region;
}

void region_t::determine_tile_standability(const int &x, const int &y, const int &z) {
	const int index = idx(x,y,z);
	const int index_above = idx(x,y,z+1);
	const bool solid = tiles[index].flags.test(tile_flags::SOLID);
	const bool above_solid = tiles[index_above].flags.test(tile_flags::SOLID);

	// TODO: This isn't right!
	if (solid) {
		tiles[index].flags.reset(tile_flags::CAN_STAND_HERE);
	} else {
		tiles[index].flags.set(tile_flags::CAN_STAND_HERE);
	}

	// You can stand on stairs
	if (tiles[index].flags.test(tile_flags::CONSTRUCTION)) {
		const int content_idx = tiles[index].contents;
		auto finder = tile_contents.find(tiles[index].contents);
		if (finder != tile_contents.end()) {
			if (finder->second.stairs != 0) tiles[index].flags.set(tile_flags::CAN_STAND_HERE);
		}
	}
}

void region_t::determine_tile_connectivity(const int &x, const int &y, const int &z) {
	const int index = idx(x,y,z);
	const bool solid = tiles[index].flags.test(tile_flags::SOLID);
	if (solid) {
		// It's solid - so we can't go anywhere!
		tiles[index].flags.reset(tile_flags::CAN_GO_NORTH);
		tiles[index].flags.reset(tile_flags::CAN_GO_NORTH_EAST);
		tiles[index].flags.reset(tile_flags::CAN_GO_EAST);
		tiles[index].flags.reset(tile_flags::CAN_GO_SOUTH_EAST);
		tiles[index].flags.reset(tile_flags::CAN_GO_SOUTH);
		tiles[index].flags.reset(tile_flags::CAN_GO_SOUTH_WEST);
		tiles[index].flags.reset(tile_flags::CAN_GO_WEST);
		tiles[index].flags.reset(tile_flags::CAN_GO_NORTH_WEST);
		tiles[index].flags.reset(tile_flags::CAN_GO_UP);
		tiles[index].flags.reset(tile_flags::CAN_GO_DOWN);
		tiles[index].flags.reset(tile_flags::CAN_STAND_HERE);
	} else {
		// Check each of the directions to see if the destination is open
		if (y>1 && tiles[idx(x,y-1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_NORTH);
		if (y<REGION_HEIGHT-1 && tiles[idx(x,y+1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_SOUTH);
		if (x<REGION_WIDTH-1 && tiles[idx(x+1,y,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_EAST);
		if (x>1 && tiles[idx(x-1,y,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_WEST);
		if (y>1 && x>1 && tiles[idx(x-1,y-1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_NORTH_WEST);
		if (y>1 && x<REGION_WIDTH-1 && tiles[idx(x+1,y-1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_NORTH_EAST);
		if (y<REGION_HEIGHT-1 && x<REGION_WIDTH-1 && tiles[idx(x+1,y+1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_SOUTH_EAST);
		if (y<REGION_HEIGHT-1 && x>1 && tiles[idx(x-1,y+1,z)].flags.test(tile_flags::CAN_STAND_HERE)) 
			tiles[index].flags.set(tile_flags::CAN_GO_SOUTH_WEST);

		// Stairs gain can_go flags based on type
		if (tiles[index].flags.test(tile_flags::CONSTRUCTION)) {
			const int content_idx = tiles[index].contents;
			auto finder = tile_contents.find(tiles[index].contents);
			if (finder != tile_contents.end()) {
				if ((finder->second.stairs == 1 || finder->second.stairs == 2)) tiles[index].flags.set(tile_flags::CAN_GO_UP);
				if ((finder->second.stairs == 1 || finder->second.stairs == 3)) tiles[index].flags.set(tile_flags::CAN_GO_DOWN);
			}
		}
	}
}

void region_t::determine_connectivity() {
	for (int z=0; z<REGION_DEPTH-1; ++z) {
		for (int y=0; y<REGION_HEIGHT; ++y) {
			for (int x=0; x<REGION_WIDTH; ++x) {
				determine_tile_standability(x, y, z);
			}
		}
	}

	for (int z=0; z<REGION_DEPTH; ++z) {
		for (int y=0; y<REGION_HEIGHT; ++y) {
			for (int x=0; x<REGION_WIDTH; ++x) {
				determine_tile_connectivity(x, y, z);
				//if (tiles[idx(x,y,z)].flags.test(tile_flags::CONSTRUCTION))
				//	std::cout << tiles[idx(x,y,z)].flags << "\n";
			}
		}
	}
}
