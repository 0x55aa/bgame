#include "settler_ai_system.h"
#include "../game.h"
#include "../world/tables.h"
#include "../messages/power_consumed_message.h"
#include "../messages/chat_emote_message.h"
#include "../tasks/help_wanted.h"
#include "../tasks/path_finding.h"
#include "../tasks/find_nearest_provision.h"
#include "../raws/raws.h"
#include "../world/skill_test.h"
#include "../../engine/geometry.hpp"
#include "../world/planet.hpp"
#include "../world/universe.hpp"
#include "../world/tile_types.hpp"
#include <map>

namespace settler_ai_detail
{

void consume_power(const int &quantity)
{
	game_engine->messaging->add_message<power_consumed_message>(power_consumed_message(quantity));
}

void emote(const string &message, const position_component3d * pos, const chat_emote_color_t &color)
{
	game_engine->messaging->add_message<chat_emote_message>(chat_emote_message(string(" ") + message, pos->pos.x + 1, pos->pos.y, pos->pos.z, color));
}

void emite_particle(const string &message, const position_component3d * pos)
{
	game_engine->messaging->add_message<particle_message>( particle_message(string(" ") + message, pos->pos.x + 1, pos->pos.y,
					pos->pos.z));
}

void append_name(stringstream &ss, const settler_ai_component &settler)
{
	ss << "@CYAN@" << settler.first_name << " " << settler.last_name << "@WHITE@ (" << settler.profession_tag << ") ";
}

string announce(const string &state, const settler_ai_component &settler)
{
	stringstream announce;
	append_name(announce, settler);
	announce << state;
	return announce.str();
}

struct settler_needs
{
	bool needs_food;
	bool needs_drink;
	bool needs_sleep;
};

settler_needs needs_clocks(settler_ai_component &settler)
{
	if (settler.state_major != SLEEPING)
	{
		settler.calories -= (settler.calorie_burn_at_rest * 3);
		settler.thirst -= 4;
		settler.wakefulness -= 6;
		if (settler.calories < 0)
			settler.calories = 0;
		if (settler.thirst < 0)
			settler.thirst = 0;
		if (settler.wakefulness < 0)
			settler.wakefulness = 0;
	}

	settler_needs result
	{ false, false, false };

	if (settler.calories < 300)
	{
		result.needs_food = true;
	}
	if (settler.thirst < 10)
	{
		result.needs_drink = true;
	}
	if (settler.wakefulness < 10)
	{
		result.needs_sleep = true;
	}
	return result;
}

bool is_move_possible(const position_component3d * pos, const int &delta_x,
		const int &delta_y, const int &delta_z)
{
	const int nx = pos->pos.x + delta_x;
	const int ny = pos->pos.y + delta_y;
	const int nz = pos->pos.z + delta_z;
	const int idx = get_tile_index(nx, ny, nz);

	if (nx < 0)
	{
		return false;
	}
	if (nx > REGION_WIDTH - 1)
	{
		return false;
	}
	if (ny < 0)
	{
		return false;
	}
	if (ny > REGION_HEIGHT - 1)
	{
		return false;
	}
	if (nz < 1)
	{
		return false;
	}
	if (nz > REGION_DEPTH - 1)
	{
		return false;
	}
	tile_t * my_tile = world::planet->get_tile(pos->pos);
	if (delta_x < 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_WEST))
		return false;
	if (delta_x > 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_EAST))
		return false;
	if (delta_y < 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_SOUTH))
		return false;
	if (delta_y > 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_NORTH))
		return false;
	if (delta_z < 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_DOWN))
		return false;
	if (delta_z > 0 and !my_tile->flags.test(TILE_OPTIONS::CAN_GO_UP))
		return false;

	if (world::planet->get_region(pos->pos.region)->tiles[idx].flags.test(
			TILE_OPTIONS::WALK_BLOCKED))
	{
		return false;
	}

	return true;
}

inline void move_to(position_component3d * pos, const int &nx, const int &ny,
		const int &nz)
{
	facing_t new_facing = OMNI;
	if (pos->pos.x < nx)
	{
		new_facing = EAST;
	}
	if (pos->pos.x > nx)
	{
		new_facing = WEST;
	}
	if (pos->pos.y < ny)
	{
		new_facing = SOUTH;
	}
	if (pos->pos.y > ny)
	{
		new_facing = NORTH;
	}

	pos->moved = true;
	pos->pos.x = nx;
	pos->pos.y = ny;
	pos->pos.z = nz;
	pos->facing = new_facing;
	game_engine->messaging->add_message<entity_moved_message>(
			entity_moved_message());
}

// FIXME: This won't work properly
void wander_randomly(settler_ai_component &settler, position_component3d * pos)
{
	// For now, they will wander around aimlessly with no purpose or collision-detection.
	int x = pos->pos.x;
	int y = pos->pos.y;
	int z = pos->pos.z;

	int direction = game_engine->rng.roll_dice(1, 6);
	switch (direction)
	{
	case 1:
		if (is_move_possible(pos, -1, 0, 0))
		{
			move_to(pos, x - 1, y, z);
		}
		break;
	case 2:
		if (is_move_possible(pos, 1, 0, 0))
		{
			move_to(pos, x + 1, y, z);
		}
		break;
	case 3:
		if (is_move_possible(pos, 0, -1, 0))
		{
			move_to(pos, x, y - 1, z);
		}
		break;
	case 4:
		if (is_move_possible(pos, 0, 1, 0))
		{
			move_to(pos, x, y + 1, z);
		}
		break;
	case 5:
		if (is_move_possible(pos, 0, 0, 1))
		{
			move_to(pos, x, y, z + 1);
		}
		break;
	case 6:
		if (is_move_possible(pos, 0, 0, -1))
		{
			move_to(pos, x, y, z - 1);
		}
		break;
	}
}

int create_needs_fulfillment_job(const int &need,
		settler_ai_component * settler, position_component3d * pos)
{
	int chosen_source_id = ai::find_nearest_provision(need, pos->pos.x,
			pos->pos.y, pos->pos.z);
	if (chosen_source_id == -1)
	{
		// TODO: Physical effects of missing these needs
		switch (need)
		{
		case 1:
		{
			emote("THERE IS NO FOOD!", pos, RED);
			settler->calories += 500;
		}
			break;
		case 2:
		{
			emote("THERE IS NO CLEAN WATER!", pos, RED);
			settler->thirst += 100;
		}
			break;
		case 3:
		{
			emote("THERE IS NOWHERE TO SLEEP!", pos, RED);
			settler->wakefulness += 100;
		}
			break;
		}
		settler->state_major = IDLE;
		return -1;
	}
	else
	{
		/*
		switch (need)
		{
		case 1:
		{
			emote("I'm hungry, Cordex!", pos, RED);
		}
			break;
		case 2:
		{
			emote("I'm thirsty, Cordex!", pos, RED);
		}
			break;
		case 3:
		{
			emote("I'm tired!", pos, RED);
		}
			break;
		}
		*/
	}

	if (need == 3)
	{
		// Claim the tent!
		provisions_component * bed = ECS->find_entity_component<provisions_component>(chosen_source_id);
		bed->provides_quantity = 1;
	}

	ai::job_t job;
	job.assigned_to = settler->entity_id;
	job.current_step = 0;
	job.job_id = ai::get_next_job_id();
	job.type = ai::MEET_PHYSICAL_NEED;

	position_component3d * source_pos = ECS->find_entity_component<position_component3d>(chosen_source_id);
	job.steps.push_back(ai::job_step_t{ ai::MOVE_TO, source_pos->pos.x, source_pos->pos.y, source_pos->pos.z, chosen_source_id, false, "", 0 });
	job.steps.push_back(ai::job_step_t{ ai::CONSUME_NEED, source_pos->pos.x, source_pos->pos.y, source_pos->pos.z,chosen_source_id, false, "", need });
	ai::jobs_board[job.job_id] = job;
	return job.job_id;
}

void idle(settler_ai_component &settler, game_stats_component * stats,
		renderable_component * renderable, position_component3d * pos)
{
	bool claimed_job = false;
	for (auto it = ai::jobs_board.begin(); it != ai::jobs_board.end(); ++it)
	{
		if (it->second.assigned_to == 0 and !claimed_job)
		{
			settler.state_major = JOB;
			it->second.assigned_to = settler.entity_id;
			settler.job_id = it->second.job_id;
			claimed_job = true;
			settler.state_timer = 0;
		}
	}

	if (settler.state_major == IDLE)
	{
		renderable->glyph = '@';
		renderable->foreground = color_t
		{ 255, 255, 0 };
		settler_ai_detail::wander_randomly(settler, pos);
	}
}

void do_your_job(settler_ai_component &settler, game_stats_component * stats,
		renderable_component * renderable, position_component3d * pos)
{
	auto job = ai::jobs_board.find(settler.job_id);
	if (job == ai::jobs_board.end())
	{
		settler.state_major = IDLE;
		return;
	}
	if (job->second.current_step > job->second.steps.size() - 1)
	{
		// Job complete
		ai::jobs_board.erase(settler.job_id);
		settler.job_id = 0;
		settler.state_major = IDLE;
		return;
	}

	ai::job_step_t step = job->second.steps[job->second.current_step];
	//std::cout << "Job step #" << job->second.current_step << " of type "  << step.type << "\n";
	switch (step.type)
	{
	case ai::MOVE_TO:
	{
		// Are we there yet?
		//const int distance = std::sqrt ( ( std::abs ( pos->x - step.target_x ) *std::abs ( pos->x - step.target_x ) ) + ( std::abs ( pos->y - step.target_y ) * ( std::abs ( pos->y - step.target_y ) ) ) );
		const int distance = geometry::distance3d(pos->pos.x, pos->pos.y,
				pos->pos.z, step.target_x, step.target_y, step.target_z);
		//std::cout << "Distance: " << distance << "\n";
		if (distance <= 1)
		{
			// We've reached our destination
			++job->second.current_step;
			settler.current_path.reset();
			return;
		}

		// No - so we better consult the map
		if (settler.current_path == nullptr)
		{
			// If there is no path - request one to the destination and exit (no moving and pathing!)
			//std::cout << "Path requested\n";
			settler.current_path = ai::find_path(pos->pos, location_t
			{ pos->pos.region, step.target_x, step.target_y, step.target_z });
			//if ( settler.current_path == nullptr) std::cout << "But returned null!\n";
			return;
		}
		else
		{
			// There is a path...
			if (settler.current_path->destination.x != step.target_x
					or settler.current_path->destination.y != step.target_y
					or settler.current_path->destination.z != step.target_z)
			{
				// Does it go to the right place? If not, then make a new one and exit
				//std::cout << "Path cancelled: not the right destination.\n";
				settler.current_path.reset();
				return;
			}
			// ... and it goes to the right place!
			if (settler.current_path->steps.empty())
			{
				//std::cout << "Path cancelled: empty\n";
				settler.current_path.reset();
				return;
			}
			location_t next_step = settler.current_path->steps.front();
			settler.current_path->steps.pop();
			//game_engine->messaging->add_message<highlight_message>(highlight_message(pos->pos.x, pos->pos.y, pos->pos.z));
			//std::cout << "move from " << pos->pos.x << "," << pos->pos.y << " TO " << next_step.x << "," << next_step.y << "\n"; 
			move_to(pos, next_step.x, next_step.y, next_step.z);
		}

	}
		break;
	case ai::PICK_UP_COMPONENT:
	{
		// We've reached the component, so we pick it up
		engine::entity * item = ECS->get_entity_by_handle(step.component_id);
		item_storage_component * storage = ECS->find_entity_component<item_storage_component>(step.component_id);
		if (storage != nullptr)
		{
			storage->deleted = true; // It's not stored anymore, so delete the component
		}
		position_component3d * item_pos = ECS->find_entity_component<position_component3d>(step.component_id);
		if (item_pos != nullptr)
		{
			item_pos->deleted = true;
		}
		ECS->add_component<item_carried_component>(*item,item_carried_component(settler.entity_id, 0));
		game_engine->messaging->add_message<item_changed_message>(item_changed_message(item->handle));
		game_engine->messaging->add_message<entity_moved_message>(entity_moved_message());
		++job->second.current_step;
	}
		break;
	case ai::DROP_OFF_COMPONENT:
	{
		engine::entity * item = ECS->get_entity_by_handle(step.component_id);
		item_carried_component * carried = ECS->find_entity_component<item_carried_component>(step.component_id);
		carried->deleted = true; // It's not carried anymore, so delete the component
		ECS->add_component<position_component3d>(*item,position_component3d(pos->pos, OMNI));
		game_engine->messaging->add_message<item_changed_message>(item_changed_message(item->handle));
		game_engine->messaging->add_message<entity_moved_message>(entity_moved_message());
		++job->second.current_step;
	}
		break;
	case ai::DROP_OFF_TOOL:
	{
		engine::entity * item = ECS->get_entity_by_handle(step.component_id);
		item_carried_component * carried = ECS->find_entity_component<item_carried_component>(step.component_id);
		carried->deleted = true; // It's not carried anymore, so delete the component
		ECS->add_component<position_component3d>(*item,position_component3d(pos->pos, OMNI));
		item_component * item_c = ECS->find_entity_component<item_component>(step.component_id);
		item_c->claimed = false;
		game_engine->messaging->add_message<item_changed_message>(item_changed_message(item->handle));
		game_engine->messaging->add_message<entity_moved_message>(entity_moved_message());
		++job->second.current_step;
	} break;
	case ai::DIG_TILE:
	{
		tile_t * target = world::planet->get_tile(location_t{pos->pos.region, step.target_x, step.target_y, step.target_z});
		target->flags.reset(TILE_OPTIONS::SOLID);
		target->flags.reset(TILE_OPTIONS::VIEW_BLOCKED);
		target->flags.reset(TILE_OPTIONS::WALK_BLOCKED);
		target->flags.set(TILE_OPTIONS::CAN_STAND_HERE);
		target->base_tile_type = tile_type::FLAT;
		target->covering = tile_covering::BARE;
		game_engine->messaging->add_message<walkability_changed_message> ( walkability_changed_message () );
		++job->second.current_step;
	} break;
	case ai::CONSTRUCT_WITH_SKILL:
	{
		++settler.state_timer;
		if (settler.state_timer > step.required_skill_difficulty)
		{
			settler.state_timer = 0;
			//std::cout << "Skill roll requested: " << step.skill_name << " of difficulty " << +step.required_skill_difficulty << "\n";
			auto result = game_system::skill_roll(settler.entity_id,
					step.skill_name, step.required_skill_difficulty);
			if (result >= game_system::SUCCESS)
			{
				++job->second.current_step;
				emote("Work Complete!", pos, chat_emote_color_t::BLUE);
			}
			else
			{
				if (result == game_system::CRITICAL_FAIL)
				{
					emote("OUCH!", pos, chat_emote_color_t::RED);
					int damage_taken = game_engine->rng.roll_dice(1, 6);
					game_engine->messaging->add_message<inflict_damage_message>(
							inflict_damage_message(settler.entity_id,
									damage_taken,
									step.skill_name + string(" Mishap"),
									pos->pos.x, pos->pos.y, pos->pos.z));
				}
				else
				{
					int random = game_engine->rng.roll_dice(1, 6);
					switch (random)
					{
					case 1:
						emote("*working*", pos, chat_emote_color_t::BLUE);
						break;
					case 2:
						emote("This is harder than it looks!", pos,
								chat_emote_color_t::BLUE);
						break;
					case 3:
						emote("That didn't work, I'll try again.", pos,
								chat_emote_color_t::BLUE);
						break;
					case 4:
						emote("Do I really know what I'm doing?", pos,
								chat_emote_color_t::BLUE);
						break;
					case 5:
						emote("That wasn't what I expected to happen!", pos,
								chat_emote_color_t::BLUE);
						break;
					case 6:
						emote("Oh dear, trying again", pos,
								chat_emote_color_t::BLUE);
						break;
					}
				}
			}
		}
	}
		break;
	case ai::CONVERT_PLACEHOLDER_STRUCTURE:
	{
		debug_name_component * name = ECS->find_entity_component<
				debug_name_component>(step.placeholder_structure_id);
		ECS->delete_entity(step.placeholder_structure_id);
		raws::create_structure_from_raws(name->debug_name, location_t
		{ pos->pos.region, step.target_x, step.target_y, step.target_z });
		game_engine->messaging->add_message<walkability_changed_message>(
				walkability_changed_message());
		game_engine->messaging->add_message<entity_moved_message>(
				entity_moved_message());
		++job->second.current_step;
	}
		break;
	case ai::DESTROY_COMPONENT:
	{
		game_engine->messaging->add_message<item_changed_message>(
				item_changed_message(step.component_id));
		game_engine->messaging->add_message<entity_moved_message>(
				entity_moved_message());
		ECS->delete_entity(step.component_id);
		++job->second.current_step;
	}
		break;
	case ai::CONSUME_NEED:
	{
		if (step.placeholder_structure_id == 3)
		{
			// Sleep mode
			pos->pos.x = step.target_x;
			pos->pos.y = step.target_y;
			pos->pos.z = step.target_z;
			const int wakeful_gain = stat_modifier(stats->constitution) + 8
					+ game_engine->rng.roll_dice(1, 6);
			settler.wakefulness += wakeful_gain;
			if (game_engine->rng.roll_dice(1, 3) == 1)
				emite_particle("z", pos);
			if (settler.wakefulness > 1000)
			{
				emote("YAWN!", pos, YELLOW);
				// We need to free the bed
				provisions_component * bed = ECS->find_entity_component<
						provisions_component>(step.component_id);
				bed->provides_quantity = 0;
				++job->second.current_step;
			}
		}
		else
		{
			provisions_component * provider = ECS->find_entity_component<
					provisions_component>(step.component_id);
			consume_power(provider->power_drain);
			emote(provider->action_name, pos, CYAN);
			if (step.placeholder_structure_id == 2)
			{
				settler.thirst += provider->provides_quantity;
			}
			else
			{
				settler.calories += provider->provides_quantity;
			}
			++job->second.current_step;
		}
	}
		break;
	case ai::CREATE_WOOD:
	{
		const int number_of_logs = game_engine->rng.roll_dice(1, 6);
		for (int i = 0; i < number_of_logs; ++i)
		{
			int wood_id = raws::create_item_from_raws("Wood Logs");
			ECS->add_component<position_component3d>(*ECS->get_entity_by_handle(wood_id), position_component3d(location_t{ pos->pos.region, step.target_x, step.target_y,	step.target_z }, OMNI));
			game_engine->messaging->add_message<item_changed_message>(item_changed_message(wood_id));
		}
		++job->second.current_step;
	}
		break;
	case ai::CREATE_ITEM:
	{
		int wood_id = raws::create_item_from_raws(step.skill_name);
		ECS->add_component<position_component3d>(
				*ECS->get_entity_by_handle(wood_id),
				position_component3d(
						location_t
						{ pos->pos.region, step.target_x, step.target_y,
								step.target_z }, OMNI));
		game_engine->messaging->add_message<item_changed_message>(
				item_changed_message(wood_id));
		++job->second.current_step;
	}
		break;
	case ai::CONSUME_POWER:
	{
		consume_power(step.component_id);
		++job->second.current_step;
	}
		break;
	}
}

}

void settler_ai_system::tick(const double &duration_ms)
{
	if (universe->globals.paused)
	{
		return;
	}

	// Obtain a link to the calendar
	calendar_component * calendar = ECS->find_entity_component<
			calendar_component>(universe->globals.cordex_handle);

	vector<settler_ai_component> * settlers = ECS->find_components_by_type<
			settler_ai_component>();
	for (settler_ai_component &settler : *settlers)
	{
		if (settler.next_tick > calendar->system_time)
		{
			break;
		}

		settler_ai_detail::settler_needs needs =
				settler_ai_detail::needs_clocks(settler);
		position_component3d * pos = ECS->find_entity_component<
				position_component3d>(settler.entity_id);

		// Needs will override current action!
		if (needs.needs_sleep and settler.state_major != JOB)
		{
			settler.state_major = JOB;
			settler.job_id = settler_ai_detail::create_needs_fulfillment_job(3,
					&settler, pos);
		}
		else if (needs.needs_drink and settler.state_major != JOB)
		{
			settler.state_major = JOB;
			settler.job_id = settler_ai_detail::create_needs_fulfillment_job(2,
					&settler, pos);
		}
		else if (needs.needs_food and settler.state_major != JOB)
		{
			settler.state_major = JOB;
			settler.job_id = settler_ai_detail::create_needs_fulfillment_job(1,
					&settler, pos);
		}

		// Perform actions
		renderable_component * renderable = ECS->find_entity_component<
				renderable_component>(settler.entity_id);
		game_stats_component * stats = ECS->find_entity_component<
				game_stats_component>(settler.entity_id);

		if (settler.next_tick <= calendar->system_time)
		{
			// Time for the settler to do something!
			if (settler.state_major == IDLE)
			{
				settler_ai_detail::idle(settler, stats, renderable, pos);
			}
			else if (settler.state_major == JOB)
			{
				settler_ai_detail::do_your_job(settler, stats, renderable, pos);
			}

			// Random pause
			int initiative_roll = 10
					- (game_engine->rng.roll_dice(1, 6)
							+ stat_modifier(stats->dexterity));
			if (initiative_roll < 1)
				initiative_roll = 1;
			if (initiative_roll > 10)
				initiative_roll = 10;
			settler.next_tick = calendar->system_time + initiative_roll;
		}
	}
}
