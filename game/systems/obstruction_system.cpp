#include "obstruction_system.h"
#include "../components/obstruction_component.h"
#include "../components/position_component.h"
#include "../world/world.h"
#include "../../engine/globals.h"

void engine::ecs::obstruction_system::tick(const double& duration_ms)
{
    world::walk_blocked.clear();
    world::view_blocked.clear();

    const vector<obstruction_component> * blockages = engine::globals::ecs->find_components_by_type<obstruction_component>();
    for (const obstruction_component &block : *blockages) {
      
        position_component * pos = engine::globals::ecs->find_entity_component<position_component>(block.entity_id);
        if (pos != nullptr) {
            const int idx = world::current_region->idx ( pos->x, pos->y );
	    if (block.blocks_entry) world::walk_blocked[idx] = true;
	    if (block.blocks_visibility) world::view_blocked[idx] = true;
        }
    }
}
