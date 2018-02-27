#include "chunk_depth_shader.hpp"

chunk_depth_shader_t::chunk_depth_shader_t() {
	load_shader("game_assets/shaders45/chunkshader_depth_vertex.glsl", "game_assets/shaders45/chunkshader_depth_fragment.glsl");

	view_matrix = get_uniform("view_matrix");
	projection_matrix = get_uniform("projection_matrix");
}
