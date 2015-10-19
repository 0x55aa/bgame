#pragma once

#include "entity.h"

namespace engine {
namespace ecs {

entity make_test_entity(const int &x, const int &y);
entity make_camera_entity();
entity make_cordex(const int &x, const int &y, const long &system_time);
  
}
}
