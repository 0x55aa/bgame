/*
 * Provides functionality relating to regions. This is an API - it hides the actual details.
 */
#pragma once

#include <vector>
#include "../tile.hpp"
#include "../constants.hpp"
#include "../../bengine/bitset8.hpp"
#include "../../components/position.hpp"
#include "../indices.hpp"
#include <functional>

/*
 * Region code is inside this namespace.
 */
namespace region {

    /*************************************
    * Creating regions
    */

    /* Create a new region. */
    void new_region(const int x, const int y, const std::size_t biome);

    /* Zero the region. */
    void zero_map();

    /*************************************
     * Serialization - load/save
     */

    /* Save the current region to disk. */
    void save_current_region();

    /* Load the current region from disk, using the specified world co-ordinates. */
    void load_current_region(const int region_x, const int region_y);

    /*************************************
     * Utility information
     */

    /* Find ground-level (Z) for a given X/Y pair */
    int ground_z(const int x, const int y);

    /* Get the X co-ordinate of the region. */
    int region_x();

    /* get the Y co-ordinate of the region. */
    int region_y();

    /* Get the biome index of the current region. */
    std::size_t get_biome_idx();

    /*************************************
     * Tile Type and Material
     */

    /* Retrieve the type of tile present at a cell. */
    uint8_t tile_type(const int idx);

    /* Set the tile type of a cell. */
    void set_tile_type(const int idx, const uint8_t type);

    /* Retrieve the material for a cell. */
    std::size_t material(const int idx);

    /* How many hit points does a cell have? */
    uint8_t tile_hit_points(const int idx);

    /* Set the tile material for a cell. */
    void set_tile_material(const int idx, const std::size_t material);

    /* Apply damage to a tile. */
    void damage_tile(const int idx, const uint8_t damage);

    /*************************************
     * Tile Flags
     */

    /* Is a flag set for a tile? */
    bool flag(const int idx, uint8_t flag);

    /* Is a cell solid? */
    bool solid(const int idx);

    /* Is a cell opaque? */
    bool opaque(const int idx);

    /* Is a cell revealed (has been visible at some point)? */
    bool revealed(const int idx);

    /* Is the cell above ground? */
    bool above_ground(const int idx);

    /* Is there a blood stain here? */
    bool blood_stain(const int idx);

    /* Reveal a cell. */
    void reveal(const int idx);

    /* Make a cell visible. */
    void make_visible(const int idx);

    /* Set the state of the SOLID flag for a cell. */
    void set_solid(const int idx, bool val);

    /* Set the state for the OPAQUE flag for a cell. */
    void set_opaque(const int idx, bool val);

    /* Set a flag for the cell. */
    void set_flag(const int idx, const uint8_t flag);

    /* Unset a flag for the cell. */
    void reset_flag(const int idx, const uint8_t flag);

    /* Set the bloodstain flag for a cell. */
    void set_bloodstain(const int idx, const bool val);

    /* Clear the visibility map. */
    void clear_visibility();

    /*************************************
     * Water
     */

    /* Get the water level at a specified cell */
    uint8_t water_level(const int idx);

    /* Set the water level for a cell. */
    void set_water_level(const int idx, const uint8_t level);

    /* Add water to a cell. */
    void add_water(const int idx);

    /* Remove 1 level of water from a cell. */
    void remove_water(const int idx);

    /*************************************
     * Vegetation
     */

    /* Retrieve the vegetation type at a cell. */
    std::size_t veg_type(const int idx);

    /* Retrieve the vegetation ticker (lifecycle counter) for a cell. */
    uint16_t veg_ticker(const int idx);

    /* Retrieve the current vegetation lifecycle indicator for a cell. */
    uint8_t  veg_lifecycle(const int idx);

    /* How many hit points does the vegetation layer have? */
    uint8_t veg_hp(const int idx);

    /* Set the vegetation type index for a cell. */
    void set_veg_type(const int idx, const uint8_t type);

    /* Set the vegetation hit points for a cell. */
    void set_veg_hp(const int idx, const uint8_t hp);

    /* Set the vegetation tick counter for a cell. */
    void set_veg_ticker(const int idx, const uint16_t ticker);

    /* Set the vegetation lifecycle state for a cell. */
    void set_veg_lifecycle(const int idx, const uint8_t lifecycle);

    /* Apply damage to a tile's vegetation. */
    void damage_vegetation(const int idx, const uint8_t damage);

    /*************************************
     * Stockpiles
     */

    /* Get the stockpile ID # for the cell. */
    std::size_t stockpile_id(const int idx);

    /* Set the Stockpile ID # for a cell. */
    void set_stockpile_id(const int idx, const std::size_t id);

    /* Erase a stockpile by ID #. */
    void delete_stockpile(const std::size_t stockpile_id);

    /*************************************
     * Bridges
     */

    /* Get the Bridge ID # for the cell. */
    std::size_t bridge_id(const int idx);

    /* Set the bridge ID # for a cell. */
    void set_bridge_id(const int idx, const std::size_t id);

    /* Erase a bridge by ID #. */
    void delete_bridge(const std::size_t bridge_id);

    /* For each bridge, execute... */
    void each_bridge(const std::function<void(std::size_t)> &func);

    /*************************************
     * Trees
     */

    /* Get the ID # of a tree on this cell. */
    std::size_t tree_id(const int idx);

    /* Get the next tree ID */
    std::size_t next_tree_id();

    /* Set the tree ID # for a cell. */
    void set_tree_id(const int idx, const std::size_t tree_id);

    /* Erase a tree by ID #. */
    void delete_tree(const std::size_t tree_id);

    /* Increment the tree counter. */
    void inc_next_tree();

    /* Recalculate all tiles. */
    void tile_recalc_all();

    /* Recalculate render information for a single tile. */
    void calc_render(const int idx);

    /* Recalculate everything about a single tile. */
    void tile_calculate(const int x, const int y, const int z);

    /*************************************
     * Builders
     */

    /* Set lots of details of a tile at once. */
    void set_tile(const int idx, const uint8_t type, const bool solid, const bool opaque,
                  const std::size_t material, const uint8_t water, const bool remove_vegetation = true,
                  const bool construction = true);

    /* Mark a cell as open space. */
    void make_open_space(const int idx);

    /* Mark a cell as a floor. */
    void make_floor(const int idx);

    /* Mark a cell as a ramp. */
    void make_ramp(const int idx);

    /* Mark a cell as an up staircase. */
    void make_stairs_up(const int idx);

    /* Mark a cell as a down staircase. */
    void make_stairs_down(const int idx);

    /* Mark a cell as an updown staircase. */
    void make_stairs_updown(const int idx);

    /* Mark a cell as a wall. */
    void make_wall(const int idx, const std::size_t mat);

    /* Run a function on every tile. */
    void for_all_tiles(const std::function<void(int)> &func);
}