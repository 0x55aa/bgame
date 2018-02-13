#include "ai_stuck.hpp"
#include "../../components/ai_tags/ai_tag_my_turn.hpp"
#include "../../components/position.hpp"
#include "../../components/settler_ai.hpp"
#include "../../components/buildings/construct_provides_sleep.hpp"
#include "../../components/logger.hpp"
#include "../gui/log_system.hpp"
#include "../physics/movement_system.hpp"
#include "../../planet/region/region.hpp"

namespace systems {
	namespace ai_stuck {
		inline bool is_stuck_or_invalid(const position_t &pos) {
			const int map_index = mapidx(pos.x, pos.y, pos.z);
			return ((map_index < 0 || map_index >(REGION_HEIGHT*REGION_WIDTH*REGION_DEPTH)) || region::solid(map_index) ||
				(
					!region::flag(map_index, CAN_GO_NORTH) &&
					!region::flag(map_index, CAN_GO_SOUTH) &&
					!region::flag(map_index, CAN_GO_EAST) &&
					!region::flag(map_index, CAN_GO_WEST) &&
					!region::flag(map_index, CAN_GO_UP) &&
					!region::flag(map_index, CAN_GO_DOWN)
					));
		}

		void run(const double &duration_ms) {
			using namespace bengine;
			each<ai_tag_my_turn_t, position_t, settler_ai_t>([](entity_t &e, ai_tag_my_turn_t &turn, position_t &pos, settler_ai_t &ai) {
				if (is_stuck_or_invalid(pos)) {
					logging::log_message msg{ LOG{}.text("Warning - settler is stuck; activating emergency teleport to bed!")->chars };
					logging::log(msg);
					std::cout << "Warning - stuck settler!\n";
					bool done = false;
					each<position_t, construct_provides_sleep_t>([&e, &pos, &done](entity_t &E, position_t &P, construct_provides_sleep_t &S) {
						if (!done) {
							movement::move_to(e, pos, P);
							done = true;
							// This should use power
						}
					});
					delete_component<ai_tag_my_turn_t>(e.id);
				}
			});
		}
	}
}