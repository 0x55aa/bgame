#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include "../position.hpp"
#include "../../bengine/ecs.hpp"

struct ai_tag_work_farm_fixsoil {

    enum fix_steps { FIND_HOE, FETCH_HOE, FIND_SOIL, FETCH_SOIL, FIND_TARGET, GOTO_TARGET, FIX_TARGET };

	ai_tag_work_farm_fixsoil() {}
	fix_steps step = FIND_HOE;
	std::size_t tool_id = 0;
	std::size_t soil_id = 0;
	std::shared_ptr<bengine::navigation_path<position_t>> current_path; // Not serialized

    template<class Archive>
    void serialize(Archive & archive)
    {
        // Nothing to save
        archive( step, tool_id, soil_id );
    }
};

CEREAL_REGISTER_TYPE(bengine::impl::component_store_t<bengine::impl::component_t<ai_tag_work_farm_fixsoil>>)