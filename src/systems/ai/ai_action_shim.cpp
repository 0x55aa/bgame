#include "ai_action_shim.hpp"
#include "../../main/game_globals.hpp"
#include "../../components/ai_mode_idle.hpp"
#include "../../components/ai_tag_my_turn.hpp"
#include "../../messages/tick_message.hpp"

void ai_action_shim::configure() {}

void ai_action_shim::update(const double duration_ms) {
    if (pause_mode != RUNNING) return;

    each<ai_tag_my_turn_t, ai_mode_idle_t>([] (entity_t &e, ai_tag_my_turn_t &t, ai_mode_idle_t &i) {
        emit_deferred(action_available_message{e.id});
        delete_component<ai_tag_my_turn_t>(e.id);
    });
}
