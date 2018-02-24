#include "design_harvest.hpp"
#include "../../global_assets/farming_designations.hpp"
#include "../../bengine/IconsFontAwesome.h"
#include "../../bengine/imgui.h"
#include "../../bengine/imgui_impl_glfw_gl3.h"
#include "../mouse.hpp"
#include "../../planet/region/region.hpp"
#include "../../raws/plants.hpp"
#include "../../raws/defs/plant_t.hpp"
#include "../../raws/items.hpp"
#include "../../raws/defs/item_def_t.hpp"
#include "../ai/distance_map_system.hpp"
#include "../../bengine/ecs.hpp"
#include "../../components/claimed_t.hpp"
#include "../../components/item_tags/item_seed_t.hpp"
#include "../../components/items/item.hpp"

using namespace tile_flags;

namespace systems {
	namespace design_harvest {
		std::set<int> cursors;
		const std::string win_harvest = std::string(ICON_FA_SHOPPING_BASKET) + " Harvesting";

		static void display_harvest() {
			using namespace region;

			std::string harvest_name = "";
			const int world_x = mouse_wx;
			const int world_y = mouse_wy;
			const int world_z = mouse_wz;

			const auto idx = mapidx(world_x, world_y, world_z);
			bool ok = true;
			if (veg_type(idx) == 0) ok = false;
			if (ok) {
				auto p = get_plant_def(veg_type(idx));
				if (p && !p->provides.empty()) {
					const std::string harvests_to = p->provides[veg_lifecycle(idx)];
					if (harvests_to == "none") {
						ok = false;
					}
					else {
						auto finder = get_item_def(harvests_to);
						if (finder != nullptr) {
							harvest_name = finder->name;
						}
					}
				}
				else {
					ok = false;
				}
			}

			if (ok && flag(idx, CAN_STAND_HERE)) {
				if (left_click) {
					bool found = false;
					for (const auto &g : farm_designations->harvest) {
						if (mapidx(g.second) == idx) found = true;
					}
					if (!found) {
						farm_designations->harvest.push_back(std::make_pair(false, position_t{ world_x, world_y, world_z }));
					}
				}
				else if (right_click) {
					farm_designations->harvest.erase(std::remove_if(
						farm_designations->harvest.begin(),
						farm_designations->harvest.end(),
						[&idx](std::pair<bool, position_t> p) { return idx == mapidx(p.second); }
					),
						farm_designations->harvest.end());
				}
			}

			
			ImGui::TextColored(ImVec4{ 1.0f, 1.0f, 0.0f, 1.0f }, "%s", "Click a tile to harvest it, right click to remove harvest status.");
			if (!harvest_name.empty()) {
				ImGui::TextColored(ImVec4{ 0.0f, 1.0f, 0.0f, 1.0f }, "%s", "Current tile will provide: ");
				ImGui::SameLine();
				ImGui::Text("%s", harvest_name.c_str());
			}
			
			for (const auto &cursor : farm_designations->harvest) {
				cursors.insert(mapidx(cursor.second));
			}
		}

		static int selected_seed = 0;

		static void display_farms() {
			using namespace bengine;
			using namespace region;

			ImGui::Text("Plant type:");
			ImGui::SameLine();

			std::map<std::string, std::pair<int, std::string>> available_seeds;
			each_without<claimed_t, item_seed_t, item_t>([&available_seeds](entity_t &e, item_seed_t &seed, item_t &item) {
				auto plant_finder = get_plant_def(get_plant_idx(seed.grows_into));
				if (plant_finder) {
					const std::string name = plant_finder->name;
					auto finder = available_seeds.find(name);
					if (finder == available_seeds.end()) {
						available_seeds[name] = std::make_pair(1, seed.grows_into);
					}
					else {
						int tmp = available_seeds[name].first;
						available_seeds[name].first = tmp + 1;
					}
				}
			});

			std::vector<std::string> seed_options;
			std::string seed_list = "";
			for (const auto &seed : available_seeds) {
				seed_list += seed.first + std::string(" (") + std::to_string(seed.second.first) + std::string(")") + (char)0;
				seed_options.push_back(seed.second.second);
				//std::cout << seed.first << " - " << seed.second.first << "\n";
			}
			seed_list += "\0";

			ImGui::Combo("##SeedSelector", &selected_seed, seed_list.c_str());
			ImGui::Text("Select tiles to designate as farms for this seed type, or right-click to remove status.");

			if (available_seeds.size() > 0) {
				const int world_x = mouse_wx;
				const int world_y = mouse_wy;
				const int world_z = mouse_wz;

				const auto idx = mapidx(world_x, world_y, world_z);
				if (flag(idx, CAN_STAND_HERE) && !flag(idx, CONSTRUCTION)) {
					if (left_click) {
						auto farm = farm_designations->farms.find(idx);
						if (farm == farm_designations->farms.end()) {
							bool found = false;
							std::size_t seed_id = 0;
							each_without<claimed_t, item_seed_t>([&found, &seed_id, &seed_options](entity_t &seed_entity, item_seed_t &seed) {
								//std::cout << seed.grows_into << " / " << seed_options[selected_seed] << "\n";
								if (!found && seed.grows_into == seed_options[selected_seed]) {
									found = true;
									seed_entity.assign(claimed_t{ 0 });
									seed_id = seed_entity.id;
								}
							});
							if (found) {
								farm_designations->farms[idx] = farm_cycle_t{ 0, seed_options[selected_seed], seed_id };
							}
						}
					}
					else if (right_click) {
						farm_designations->farms.erase(idx);
					}
				}
			}

			for (const auto &farm : farm_designations->farms) {
				cursors.insert(farm.first);
			}
		}

		static std::string harvest_tab = std::string(ICON_FA_LEAF) + " Harvest";
		static std::string farms_tab = std::string(ICON_FA_TASKS) + " Farms";

		void run(const double &duration_ms) {
			cursors.clear();
			ImGui::Begin(win_harvest.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
			//ImGui::BeginTabBar("##farming#tabs");
			//ImGui::DrawTabsBackground();

			if (ImGui::Button(harvest_tab.c_str())) {
				display_harvest();
			}
			if (ImGui::Button(farms_tab.c_str())) {
				display_farms();
			}

			//ImGui::EndTabBar();
			ImGui::End();
		}
	}
}
