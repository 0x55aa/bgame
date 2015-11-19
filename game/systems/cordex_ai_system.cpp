#include "cordex_ai_system.h"
#include <iostream>
#include "../game.h"

void cordex_ai_system::tick ( const double& duration_ms )
{
    // Handle new build orders
    vector<build_order_message> * orders = game_engine->messaging->get_messages_by_type< build_order_message >();
    for (const build_order_message &msg : *orders) {
	msg.deleted = true;
	std::cout << "Logged an order to build [" << msg.structure_name << "], at {" << msg.x << "," << msg.y << "}\n";
	
	// Create a job
	// Create the sub-tasks
	// Add it to the help-wanted board
    }
}