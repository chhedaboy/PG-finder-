#include "Hostel.h"
#include "Colleges.h"
#include "Json.h"

Hostel::Hostel(int id, int ownerId, const std::string& name, const std::string& location,
               const std::string& gender, const std::string& food)
    : Accommodation(name, location, gender, food), id(id), ownerId(ownerId) {}

int Hostel::getId() const { return id; }
int Hostel::getOwnerId() const { return ownerId; }

void Hostel::setDistance(const std::string& collegeId, double km) { distances[collegeId] = km; }

bool Hostel::distanceTo(const std::string& collegeId, double& km) const {
    auto it = distances.find(collegeId);
    if (it == distances.end()) return false;
    km = it->second;
    return true;
}

const std::map<std::string, double>& Hostel::getDistances() const { return distances; }

void Hostel::addRoom(const Room& room) { rooms.push_back(room); }

int Hostel::nextRoomId() const {
    int mx = 0;
    for (const Room& r : rooms)
        if (r.getId() > mx) mx = r.getId();
    return mx + 1;
}

Room* Hostel::findRoom(int roomId) {
    for (Room& r : rooms)
        if (r.getId() == roomId) return &r;
    return nullptr;
}

const Room* Hostel::findRoom(int roomId) const {
    for (const Room& r : rooms)
        if (r.getId() == roomId) return &r;
    return nullptr;
}

bool Hostel::removeRoom(int roomId) {
    for (size_t i = 0; i < rooms.size(); i++) {
        if (rooms[i].getId() == roomId) {
            rooms.erase(rooms.begin() + i);
            return true;
        }
    }
    return false;
}

const std::vector<Room>& Hostel::getRooms() const { return rooms; }

int Hostel::totalAvailableBeds() const {
    int n = 0;
    for (const Room& r : rooms) n += r.availableBeds();
    return n;
}

bool Hostel::checkAvailability() const { return totalAvailableBeds() > 0; }
std::string Hostel::getType() const { return "Hostel"; }

// hostels.txt line: id|ownerId|name|location|gender|food|COEP=4.2;DYP=7.8;...
std::string Hostel::toRecord() const {
    std::string d;
    for (const auto& kv : distances) {
        if (!d.empty()) d += ";";
        d += kv.first + "=" + jsonNum(kv.second);
    }
        return std::to_string(id) + "|" + std::to_string(ownerId) + "|" + name + "|" + location + "|" +
            gender + "|" + food + "|" + d;
}

std::string Hostel::toJson(const std::string& collegeId) const {
    std::string j = "{\"id\":" + std::to_string(id) +
                    ",\"ownerId\":" + std::to_string(ownerId) +
                    ",\"name\":" + jsonStr(name) +
                    ",\"location\":" + jsonStr(location) +
                    ",\"gender\":" + jsonStr(gender) +
                    ",\"food\":" + jsonStr(food) +
                    ",\"type\":" + jsonStr(getType()) +
                    ",\"available\":" + (checkAvailability() ? "true" : "false") +
                    ",\"availableBeds\":" + std::to_string(totalAvailableBeds());

    double km;
    if (!collegeId.empty() && distanceTo(collegeId, km)) j += ",\"distance\":" + jsonNum(km);

    j += ",\"distances\":{";
    bool first = true;
    for (const College& c : allColleges()) {
        if (distanceTo(c.id, km)) {
            if (!first) j += ",";
            j += jsonStr(c.id) + ":" + jsonNum(km);
            first = false;
        }
    }
    j += "},\"rooms\":[";
    for (size_t i = 0; i < rooms.size(); i++) {
        if (i) j += ",";
        j += rooms[i].toJson();
    }
    return j + "]}";
}
