// Room.h - a room with 1, 2 or 3 beds (sharing type = number of beds)
#pragma once
#include <string>
#include <vector>
#include "Bed.h"

class Room {
private:
    int id;                 // unique inside its hostel
    int sharing;            // 1, 2 or 3
    bool ac;
    int rent;               // per bed, per month (Rs)
    std::vector<Bed> beds;

public:
    Room(int id, int sharing, bool ac, int rent);
    // bedStates is a string like "010": '1' = occupied, '0' = available
    Room(int id, int sharing, bool ac, int rent, const std::string& bedStates);

    int getId() const;
    int getSharing() const;
    bool isAC() const;
    int getRent() const;
    int getCapacity() const;
    int availableBeds() const;
    const std::vector<Bed>& getBeds() const;

    void setRent(int newRent);
    void setAC(bool value);
    void setAvailableBeds(int count);     // owner: make exactly 'count' beds available
    bool bookBed(int bedId);              // student: Available -> Occupied

    std::string bedStates() const;
    std::string toRecord(int hostelId) const;
    std::string toJson() const;
};
