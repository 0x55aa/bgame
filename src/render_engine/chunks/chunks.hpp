#pragma once

#include <array>
#include "constants.hpp"
#include <bitset>
#include <atomic>
#include <boost/container/flat_map.hpp>
#include <vector>

namespace chunks {

    struct geometry_t {
        float type;
        float x;
        float y;
        float z;
        float width;
        float height;
        float texture_id;
    };

    /* Represents a single z-layer inside a chunk */
    struct layer_t {
        std::vector<geometry_t> vertices;

        unsigned int vao = 0;
        unsigned int vbo = 0;
        std::size_t n_elements = 0;
    };

    /* Represents a 32x32 segment of the world */
    struct chunk_t {
        chunk_t() {}
        chunk_t(const int &idx, const int &x, const int &y, const int &z) : index(idx), base_x(x), base_y(y), base_z(z) {}

        void setup(const int &idx, const int &x, const int &y, const int &z) {
            index = idx;
            base_x = x;
            base_y = y;
            base_z = z;
        }

        int index = 0;
        int base_x=0, base_y=0, base_z=0;
        std::bitset<CHUNK_SIZE> layer_requires_render{};
        std::array<layer_t, CHUNK_SIZE> layers;
        std::atomic<bool> ready{false};
        bool has_geometry = false;

        void update();
        void greedy_floors(boost::container::flat_map<int, unsigned int> &floors, const int &layer);
        void greedy_cubes(boost::container::flat_map<int, unsigned int> &cubes, const int &layer);
        void update_buffer();
    };

    extern std::array<chunk_t, CHUNKS_TOTAL> chunks;

    extern bool chunks_initialized;
    void initialize_chunks();
    void mark_chunk_dirty(const int &idx);
    void mark_chunk_clean(const int &idx);
    void update_dirty_chunks();
    void enqueue_vertex_update(const int &idx);
    void update_buffers();
}