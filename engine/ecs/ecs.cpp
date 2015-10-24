#include "ecs.h"
#include "base_message.h"
#include "system_factory.h"
#include "entity_factory.h"
#include <memory>
#include <queue>
#include <string>
#include <iostream>

using std::vector;
using std::unordered_map;
using std::unique_ptr;
using std::queue;
using std::bitset;
using std::string;

namespace engine {
namespace ecs {

int handle_count = 0;
unordered_map<int, entity> entities;
vector<unique_ptr<base_component>> components;
unordered_map<int, int> component_handle_index;
vector<unique_ptr<base_system>> systems;
//queue<base_message> mailbox;

/*
 * Return the next unique handle for a component. When threading is introduced,
 * this will require some locking semantics - for now it's very simple.
 */
inline int next_handle() {
    int result = handle_count;
    ++handle_count;
    return result;
}

/*
 * Adds a component to the system, attached to an entity. The entity will
 * be updated to hold a handle to the component, as well as its "types"
 * bitset updated.
 */
void add_component(entity &target, unique_ptr< base_component > component)
{
    int handle = next_handle();
    component->handle = handle;
    component->entity_id = target.handle;
    target.component_handles.push_back(std::make_pair(component->type, handle));
    target.component_types.set(component_flag(component->type));
    components.push_back(std::move(component));
}

/*
 * Adds an entity to the entity map
 */
void add_entity(const entity &target) {
    entities[target.handle] = target;
}

/*
 * Returns a non-owning pointer (or nullptr) to an entity
 * with a matching entity handle.
 */
entity * get_entity_by_handle(const int &handle) {
    auto finder = entities.find(handle);
    if (finder == entities.end()) return nullptr;
    return &finder->second;
}

/**
 * Returns a list of entity handles that match a given function.
 */
vector<int> find_entities_by_func(function<bool(const entity &e)> matcher) {
    vector<int> result;
    for (auto it = entities.begin(); it != entities.end(); ++it) {
	const bool match = matcher(it->second);
	if (match) result.push_back(it->second.handle);
    }
    return result;
}

/**
 * Return a list of entities with a bit set (i.e. entities with a type of component)
 */
vector<int> find_entities_by_bitset(const int &bit_to_check) {
    return find_entities_by_func([bit_to_check] (const entity &e) { 
	return e.component_types.test(bit_to_check);
    } );
}

/*
 * Updates the index of component handles to actual vector positions.
 */
void update_component_handle_index() {
    component_handle_index.clear();
    for (auto i=0; i<components.size(); ++i) {
        component_handle_index[components[i]->handle] = i;
    }
}

/*
 * Adds a system to the list of systems.
 */
void add_system(unique_ptr<base_system> system) {
    systems.push_back(std::move(system));
}

/*
 * Called each frame, runs all registered systems.
 */
void tick(const double duration_ms) {
    update_component_handle_index();
    for (unique_ptr<base_system> &system : systems) {
        system->tick(duration_ms);
    }
}

inline void empty_all() {
    entities.clear();
    components.clear();
    systems.clear();
}

void init() {
    empty_all();
    add_system(make_camera_system());
    add_system(make_calendar_system());
    add_system(make_obstruction_system());
    add_system(make_settler_ai_system());
    add_system(make_renderable_system());
    add_system(make_viewshed_system());
}

void done() {
    empty_all();
}

inline string get_filename() {
    return "world/saved_game.dat";
}

void load_game()
{
    const string filename = get_filename();
    fstream lbfile(filename, std::ios::in | std::ios::binary);  
    
    world::load_world_constants(lbfile);
    
    int number_of_entities = 0;
    lbfile.read ( reinterpret_cast<char *> ( &number_of_entities ), sizeof ( number_of_entities ) );
    for (int i=0; i<number_of_entities; ++i) {
	entity e = construct_entity_from_file(lbfile);
	add_entity(e);
    }
    
    int number_of_components = 0;
    lbfile.read ( reinterpret_cast<char *> ( &number_of_components ), sizeof ( number_of_components ) );
    for (int i=0; i<number_of_components; ++i) {
	unique_ptr<base_component> component = construct_component_from_file(lbfile);
	const int entity_handle = component->entity_id;
	entity * entity_ptr = get_entity_by_handle(entity_handle);
	add_component(*entity_ptr, std::move(component));
    }
}

void save_game()
{
    const string filename = get_filename();
    fstream lbfile(filename, std::ios::out | std::ios::binary);
    
    world::save_world_constants(lbfile);
    
    int number_of_entities = entities.size();
    lbfile.write ( reinterpret_cast<const char *> ( &number_of_entities ), sizeof ( number_of_entities ) );
    for (auto it = entities.begin(); it != entities.end(); ++it) {
	entity e = it->second;
	e.save(lbfile);
    }
    
    int number_of_components = components.size();
    lbfile.write ( reinterpret_cast<const char *> ( &number_of_components ), sizeof ( number_of_components ) );
    for (const unique_ptr<base_component> &c : components) {
	c->save(lbfile);
    }
}



}
}
