#pragma once

#include <cstdint>
#include <memory>

/* Base class for planetary/other body definition. */
struct base_solar_system_body_t {
};

/*
 * Represents a planet or other body orbiting a star system.
 */
struct solar_system_body_t {
    /* Index within the solar system. */
    uint8_t system_idx;
    
    /* Pointer to the appropriate planetary class. */
    std::shared_ptr<base_solar_system_body_t> body_def;
};
