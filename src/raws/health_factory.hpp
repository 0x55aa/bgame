#pragma once

#include "../components/health.hpp"

health_t create_health_component_sentient(const std::size_t &species_id, const int base_hp);
health_t create_health_component_settler(const std::string &tag, const int base_hp);
health_t create_health_component_creature(const std::string &tag);
