#include "ai_work_mining.hpp"
#include "distance_map_system.hpp"
#include "../../components/ai_tag_work_mining.hpp"
#include "../../components/ai_tag_my_turn.hpp"
#include "../../components/position.hpp"
#include "../../messages/inventory_changed_message.hpp"
#include "../../components/item.hpp"
#include "path_finding.hpp"
#include "../../components/game_stats.hpp"
#include "../../utils/telemetry.hpp"
#include "../../raws/raws.hpp"
#include "../../messages/map_dirty_message.hpp"
#include "../../messages/entity_moved_message.hpp"
#include "../../messages/inflict_damage_message.hpp"
#include "../../messages/renderables_changed_message.hpp"
#include "../../messages/perform_mining.hpp"
#include "../../components/item_stored.hpp"
#include "mining_system.hpp"

void ai_work_mining::configure() {}

void ai_work_mining::update(const double duration_ms) {
    if (pause_mode == PAUSED) return;

    each<ai_tag_work_miner, ai_tag_my_turn_t, position_t>([] (entity_t &e, ai_tag_work_miner &m, ai_tag_my_turn_t &t, position_t &pos) {
        delete_component<ai_tag_my_turn_t>(e.id); // It's not my turn anymore

        std::cout << "Entity #" << e.id << " is mining\n";

        if (m.step == ai_tag_work_miner::mining_steps::GET_PICK) {
            const auto d = pick_map.get(mapidx(pos));
            std::cout << "Get pick - distance " << d << "\n";
            if (d > MAX_DIJSTRA_DISTANCE-1) {
                // Cancel the job, we can't do it
                delete_component<ai_tag_work_miner>(e.id);
                return;
            } else if (d < 1) {
                std::cout << "Picking up pick\n";

                // Pick up the axe
                std::size_t tool_id = 0;
                each<item_t>([&tool_id, &pos] (entity_t &pick, item_t &item) {
                    if (item.category.test(TOOL_DIGGING)) return; // Not an axe

                    auto axe_pos = pick.component<position_t>();
                    if (axe_pos != nullptr && *axe_pos == pos) {
                        tool_id = pick.id;
                    } else {
                        auto stored = pick.component<item_stored_t>();
                        if (stored != nullptr) {
                            auto spos = entity(stored->stored_in)->component<position_t>();
                            if (spos != nullptr && *spos == pos) {
                                tool_id = pick.id;
                            }
                        }
                    }
                });
                if (tool_id > 0) {
                    m.current_pick = tool_id;
                    emit(pickup_item_message{m.current_pick, e.id});
                    emit(pickmap_changed_message{});
                    m.step = ai_tag_work_miner::mining_steps ::GOTO_SITE;
                } else {
                    std::cout << "We failed to find the pick\n";
                    // We've failed to get the pick!
                    delete_component<ai_tag_work_miner>(e.id);
                }
                return;
            } else {
                // Path towards the pick
                std::cout << "Moving towards pick - " << e.id << "\n";
                position_t destination = pick_map.find_destination(pos);
                emit_deferred(entity_wants_to_move_message{e.id, destination});
                return;
            }
        } else if (m.step == ai_tag_work_miner::mining_steps::GOTO_SITE) {
            std::cout << "Moving towards mining target\n";
            const auto idx = mapidx(pos.x, pos.y, pos.z);
            if (mining_map[idx]==0) {
                // We're at a minable site
                m.step = ai_tag_work_miner::mining_steps::DIG;
                return;
            }

            int current_direction = 0;
            uint8_t min_value = std::numeric_limits<uint8_t>::max();
            if (mining_map[mapidx(pos.x, pos.y-1, pos.z)] < min_value && current_region->tile_flags[idx].test(CAN_GO_NORTH)) {
                min_value = mining_map[mapidx(pos.x, pos.y-1, pos.z)];
                current_direction = 1;
            }
            if (mining_map[mapidx(pos.x, pos.y+1, pos.z)] < min_value && current_region->tile_flags[idx].test(CAN_GO_SOUTH)) {
                min_value = mining_map[mapidx(pos.x, pos.y+1, pos.z)];
                current_direction = 2;
            }
            if (mining_map[mapidx(pos.x-1, pos.y, pos.z)] < min_value && current_region->tile_flags[idx].test(CAN_GO_WEST)) {
                min_value = mining_map[mapidx(pos.x-1, pos.y, pos.z)];
                current_direction = 3;
            }
            if (mining_map[mapidx(pos.x+1, pos.y, pos.z)] < min_value && current_region->tile_flags[idx].test(CAN_GO_EAST)) {
                min_value = mining_map[mapidx(pos.x+1, pos.y, pos.z)];
                current_direction = 4;
            }
            if (mining_map[mapidx(pos.x, pos.y, pos.z-1)] < min_value && current_region->tile_flags[idx].test(CAN_GO_DOWN)) {
                min_value = mining_map[mapidx(pos.x, pos.y, pos.z-1)];
                current_direction = 5;
            }
            if (mining_map[mapidx(pos.x, pos.y, pos.z+1)] < min_value && current_region->tile_flags[idx].test(CAN_GO_UP)) {
                min_value = mining_map[mapidx(pos.x, pos.y, pos.z+1)];
                current_direction = 6;
            }

            if (current_direction == 0) {
                m.step = ai_tag_work_miner::mining_steps::DROP_TOOLS;
                return;
            }

            position_t destination = pos;
            switch (current_direction) {
                case 1 : --destination.y; break;
                case 2 : ++destination.y; break;
                case 3 : --destination.x; break;
                case 4 : ++destination.x; break;
                case 5 : --destination.z; break;
                case 6 : ++destination.z; break;
            }
            emit_deferred(entity_wants_to_move_message{e.id, destination});

            return;
        } else if (m.step == ai_tag_work_miner::mining_steps::DIG) {
            auto stats = e.component<game_stats_t>();
            if (!stats) {
                m.step = ai_tag_work_miner::mining_steps::DROP_TOOLS;
                return;
            }
            auto skill_check = skill_roll(e.id, *stats, rng, "Mining", DIFICULTY_TOUGH);
            if (skill_check >= SUCCESS) {
                // Determine the digging target from here
                // Make a skill roll, and if successful complete the action
                // When complete, move to dropping the pick
                const auto idx = mapidx(pos.x, pos.y, pos.z);
                const int target_idx = mining_targets[idx];
                const int target_operation = designations->mining[target_idx];
                call_home("mining", std::to_string(target_operation));

                if (target_operation > 0) {
                    emit(perform_mining_message{mining_targets[idx], designations->mining[target_idx], static_cast<int>(pos.x), static_cast<int>(pos.y), pos.z});
                    designations->mining.erase(target_idx);
                }
                m.step = ai_tag_work_miner::mining_steps::DROP_TOOLS;
                return;
            } else {
                // Failed!
                if (skill_check == CRITICAL_FAIL) emit_deferred(inflict_damage_message{e.id, 1, "Mining Accident"});
                return;
            }

            return;
        } else if (m.step == ai_tag_work_miner::mining_steps::DROP_TOOLS) {
            emit(drop_item_message{m.current_pick, pos.x, pos.y, pos.z});
            emit(pickmap_changed_message{});
            delete_component<ai_tag_work_miner>(e.id);
            return;
        }
    });
}