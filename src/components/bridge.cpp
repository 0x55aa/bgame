#include "bridge.hpp"

void bridge_t::to_xml(xml_node * c) {
    component_to_xml(c, std::make_pair("complete", complete));
}

void bridge_t::from_xml(xml_node * c) {
    complete = c->val<bool>("complete");
}