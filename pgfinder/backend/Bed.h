// Bed.h - one bed inside a room
#pragma once
#include <string>

class Bed {
private:
    int id;
    bool occupied;

public:
    Bed(int id, bool occupied = false);

    int getId() const;
    bool isOccupied() const;
    bool book();                  // Available -> Occupied (false if already occupied)
    void setOccupied(bool value);
    std::string toJson() const;
};
