#include "Bed.h"

Bed::Bed(int id, bool occupied) : id(id), occupied(occupied) {}

int Bed::getId() const { return id; }
bool Bed::isOccupied() const { return occupied; }

bool Bed::book() {
    if (occupied) return false;
    occupied = true;
    return true;
}

void Bed::setOccupied(bool value) { occupied = value; }

std::string Bed::toJson() const {
    return "{\"id\":" + std::to_string(id) + ",\"occupied\":" + (occupied ? "true" : "false") + "}";
}
