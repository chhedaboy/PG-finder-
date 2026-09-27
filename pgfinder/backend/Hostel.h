// Hostel.h - a hostel/PG. Inherits from Accommodation.
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Accommodation.h"
#include "Room.h"

class Hostel : public Accommodation {
private:
    int id;
    int ownerId;
    std::map<std::string, double> distances;   // college id -> km  (one entry PER COLLEGE)
    std::vector<Room> rooms;

public:
    Hostel(int id, int ownerId, const std::string& name, const std::string& location,
           const std::string& gender, const std::string& food);

    int getId() const;
    int getOwnerId() const;

    // distances
    void setDistance(const std::string& collegeId, double km);
    bool distanceTo(const std::string& collegeId, double& km) const;
    const std::map<std::string, double>& getDistances() const;

    // rooms
    void addRoom(const Room& room);
    int nextRoomId() const;
    Room* findRoom(int roomId);
    const Room* findRoom(int roomId) const;
    bool removeRoom(int roomId);
    const std::vector<Room>& getRooms() const;
    int totalAvailableBeds() const;

    // from Accommodation
    bool checkAvailability() const override;
    std::string getType() const override;

    std::string toRecord() const;                                 // hostels.txt line
    std::string toJson(const std::string& collegeId = "") const;  // full details for the website
};
