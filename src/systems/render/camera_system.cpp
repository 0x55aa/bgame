#include "camera_system.hpp"
#include "../../messages/map_dirty_message.hpp"
#include "../../messages/inputs.hpp"
#include "renderables_system.hpp"
#include "../../messages/tick_message.hpp"
#include "../../utils/gl/map_render.hpp"

uint32_t camera_glyph_cycle = 0;

void camera_system::configure() {
	system_name = "Camera";
	dirty = true;
	subscribe_mbox<camera_move_requested_message>();
	subscribe_mbox<map_dirty_message>();
	subscribe<slow_tick_message>([this](slow_tick_message &msg) {
        ++camera_glyph_cycle;
    });
}

void camera_system::update(const double duration_ms) {
	std::queue<map_dirty_message> * map_change = mbox<map_dirty_message>();
	while (!map_change->empty()) {
		dirty = true;
		map_change->pop();
	}

	// Handle camera controls
	std::queue<camera_move_requested_message> * messages = mbox<camera_move_requested_message>();
	while (!messages->empty()) {
		camera_move_requested_message e = messages->front();
		messages->pop();

		if (e.direction == 1) {
			camera_position->region_x -= e.step;
			if (camera_position->region_x < 0) camera_position->region_x = 0;
			dirty = true;
			update_clipping_rectangle();
		}
		if (e.direction == 2) {
			camera_position->region_x += e.step;
			if (camera_position->region_x > REGION_WIDTH) camera_position->region_x = REGION_WIDTH;
			dirty = true;
			update_clipping_rectangle();
		}
		if (e.direction == 3) {
			camera_position->region_y -= e.step;
			if (camera_position->region_y < 0) camera_position->region_y = 0;
			dirty = true;
			update_clipping_rectangle();
		}
		if (e.direction == 4) {
			camera_position->region_y += e.step;
			if (camera_position->region_y > REGION_HEIGHT) camera_position->region_y = REGION_HEIGHT;
			dirty = true;
			update_clipping_rectangle();
		}
		if (e.direction == 5) {
			camera_position->region_z -= e.step;
			if (camera_position->region_z < 1) camera_position->region_z = 1;
			dirty = true;
			update_clipping_rectangle();
		}
		if (e.direction == 6) {
			camera_position->region_z += e.step;
			if (camera_position->region_z > REGION_DEPTH-1) camera_position->region_z = REGION_DEPTH-1;
			dirty = true;
			update_clipping_rectangle();
		}
	}

	if (dirty || clip_left == -1) {
        update_clipping_rectangle();
        dirty = false;
        world_changed = true;
	}
}

void camera_system::update_clipping_rectangle() {
	clip_left = std::max(1, camera_position->region_x - 24);
	clip_right = std::min(REGION_WIDTH-1, camera_position->region_x + 24);
	clip_top = std::max(1, camera_position->region_y - 24);
	clip_bottom = std::min(REGION_HEIGHT-1, camera_position->region_y + 24);

	emit(map_rerender_message{});
}
