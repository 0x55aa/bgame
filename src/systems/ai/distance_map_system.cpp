#include "distance_map_system.hpp"
#include "../../components/position.hpp"
#include "../../planet/region/region.hpp"
#include "../../global_assets/game_designations.hpp"
#include "../../components/grazer_ai.hpp"
#include "../../components/corpse_harvestable.hpp"
#include "../../components/buildings/construct_provides_sleep.hpp"
#include "../../components/claimed_t.hpp"
#include "../../components/settler_ai.hpp"
#include "../../bengine/ecs.hpp"
#include "../helpers/targeted_flow_map.hpp"
#include "../../components/buildings/building.hpp"

namespace systems {
	namespace distance_map {

		using namespace dijkstra;

		std::unique_ptr<flow_maps::map_t> hunting_map;
		std::unique_ptr<flow_maps::map_t> butcher_map;
		dijkstra_map bed_map;
		dijkstra_map settler_map;
		dijkstra_map levers_map;
		dijkstra_map reachable_from_cordex;
		bool dijkstra_debug = false;

		static bool huntables_dirty = true;
		static bool butcherables_dirty = true;
		static bool beds_dirty = true;
		static bool settlers_dirty = true;
		static bool levers_dirty = true;
		static bool cordex_dirty = true;

		using namespace bengine;

		void refresh_bed_map() {
			//beds_dirty = true;
		}

		void refresh_hunting_map() {
			huntables_dirty = true;
		}

		void refresh_butcherables_map() {
			butcherables_dirty = true;
		}

		void refresh_all_distance_maps() {
			//beds_dirty = true;
			huntables_dirty = true;
			butcherables_dirty = true;
			cordex_dirty = true;
		}

		static void update_hunting_map() {
			std::vector<std::tuple<int,int>> huntables;
			each<grazer_ai, position_t>([&huntables](entity_t &e, grazer_ai &ai, position_t &pos) {
				huntables.emplace_back(std::make_tuple( mapidx(pos), mapidx(pos)));
			});
			std::cout << "Found " << huntables.size() << " huntable targets\n";
			hunting_map->fill_map(huntables);
			std::cout << "There are " << hunting_map->targets.size() << " things to kill\n";
		}

		static void update_butcher_map() {
			std::vector<std::tuple<int, int>> butcherables;
			each<corpse_harvestable, position_t>([&butcherables](entity_t &e, corpse_harvestable &corpse, position_t &pos) {
				butcherables.emplace_back(std::make_tuple(mapidx(pos), mapidx(pos)));
			});
			butcher_map->fill_map(butcherables);
		}

		void update_bed_map() {
			std::vector<int> beds;
			each_without<claimed_t, construct_provides_sleep_t, position_t>([&beds](entity_t &e, construct_provides_sleep_t &bed, position_t &pos) {
				beds.emplace_back(mapidx(pos));
			});
			bed_map.update(beds);
		}

		void update_settler_map() {
			std::vector<int> settlers;
			each<settler_ai_t, position_t>([&settlers](entity_t &e, settler_ai_t &settler, position_t &pos) {
				settlers.emplace_back(mapidx(pos));
			});
			settler_map.update(settlers);
		}

		void update_levers_map() {
			std::vector<int> targets;
			for (auto it = designations->levers_to_pull.begin(); it != designations->levers_to_pull.end(); ++it) {
				auto lever_id = *it;
				auto lever_entity = entity(lever_id);
				if (!lever_entity) break;
				auto lever_pos = lever_entity->component<position_t>();
				if (!lever_pos) break;
				targets.emplace_back(mapidx(*lever_pos));
			}
			levers_map.update(targets);
		}

		static position_t cordex_pos{ 0,0,0 };

		void update_cordex_map()
		{
			if (cordex_pos.x == 0)
			{
				bengine::each<building_t, position_t>([] (bengine::entity_t &e, building_t &b, position_t &pos)
				{
					if (b.tag == "cordex")
					{
						cordex_pos = pos;
					}
				});
			}

			if (cordex_pos.x == 0)
			{
				std::cout << "WARNING - failed to find Cordex!\n";
			}

			const std::vector<int> targets{ mapidx(cordex_pos) };
			reachable_from_cordex.update(targets);
			cordex_dirty = false;
		}

		void run(const double &duration_ms) {
			if (!hunting_map) hunting_map = std::make_unique<flow_maps::map_t>();
			if (!butcher_map) butcher_map = std::make_unique<flow_maps::map_t>();

			if (huntables_dirty) {
				update_hunting_map();
				huntables_dirty = false;
			}

			if (butcherables_dirty) {
				update_butcher_map();
				butcherables_dirty = false;
			}

			if (beds_dirty) {
				update_bed_map();
				beds_dirty = false;
			}

			if (settlers_dirty) {
				update_settler_map();
				settlers_dirty = false;
			}

			if (levers_dirty) {
				update_levers_map();
				levers_dirty = false;
			}

			if (cordex_dirty)
			{
				update_cordex_map();
			}
		}
	}
}
