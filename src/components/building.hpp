#pragma once

#include <rltk.hpp>
#include <vector>

using namespace rltk;

struct building_t {
    building_t(const std::string ntag, const int w, const int h, const std::vector<rltk::vchar> &g, const bool comp) :
        tag(ntag), width(w), height(h), glyphs(g), complete(comp) {}

    std::string tag;
    int width, height;
    std::vector<rltk::vchar> glyphs;
    bool complete = false;

	building_t() {}

	std::size_t serialization_identity = 18;

    void save(std::ostream &lbfile);
    static building_t load(std::istream &lbfile);
};