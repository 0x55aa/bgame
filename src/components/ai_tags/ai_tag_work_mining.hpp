#pragma once

#include <memory>
#include "../position.hpp"
#include "../../bengine/path_finding.hpp"

struct ai_tag_work_miner {

    enum mining_steps { GET_PICK, FETCH_PICK, GOTO_SITE, DIG };

    mining_steps step = GET_PICK;
	std::size_t tool_id = 0;

	std::shared_ptr<bengine::navigation_path<position_t>> current_path; // Not serialized

};

