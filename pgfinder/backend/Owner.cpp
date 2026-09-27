#include "Owner.h"
#include "Json.h"

Owner::Owner(int id, const std::string& name) : Person(id, name) {}

std::string Owner::getRole() const { return "Owner"; }

// owners.txt line: id|name
std::string Owner::toRecord() const {
    return std::to_string(id) + "|" + name;
}

std::string Owner::toJson() const {
    return "{\"id\":" + std::to_string(id) + ",\"name\":" + jsonStr(name) + "}";
}
