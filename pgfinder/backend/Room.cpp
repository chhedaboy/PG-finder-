#include "Room.h"

Room::Room(int id, int sharing, bool ac, int rent)
    : id(id), sharing(sharing), ac(ac), rent(rent) {
    for (int i = 1; i <= sharing; i++) beds.push_back(Bed(i, false));
}

Room::Room(int id, int sharing, bool ac, int rent, const std::string& bedStates)
    : id(id), sharing(sharing), ac(ac), rent(rent) {
    for (int i = 1; i <= sharing; i++) {
        bool occ = (i - 1 < (int)bedStates.size()) && bedStates[i - 1] == '1';
        beds.push_back(Bed(i, occ));
    }
}

int Room::getId() const { return id; }
int Room::getSharing() const { return sharing; }
bool Room::isAC() const { return ac; }
int Room::getRent() const { return rent; }
int Room::getCapacity() const { return (int)beds.size(); }
const std::vector<Bed>& Room::getBeds() const { return beds; }

int Room::availableBeds() const {
    int n = 0;
    for (const Bed& b : beds)
        if (!b.isOccupied()) n++;
    return n;
}

void Room::setRent(int newRent) { rent = newRent; }
void Room::setAC(bool value) { ac = value; }

void Room::setAvailableBeds(int count) {
    if (count < 0) count = 0;
    if (count > getCapacity()) count = getCapacity();
    for (int i = 0; i < getCapacity(); i++) beds[i].setOccupied(i >= count);
}

bool Room::bookBed(int bedId) {
    for (Bed& b : beds)
        if (b.getId() == bedId) return b.book();
    return false;
}

std::string Room::bedStates() const {
    std::string s;
    for (const Bed& b : beds) s += b.isOccupied() ? '1' : '0';
    return s;
}

// rooms.txt line: hostelId|roomId|sharing|ac|rent|bedStates
std::string Room::toRecord(int hostelId) const {
    return std::to_string(hostelId) + "|" + std::to_string(id) + "|" + std::to_string(sharing) + "|" +
           (ac ? "1" : "0") + "|" + std::to_string(rent) + "|" + bedStates();
}

std::string Room::toJson() const {
    std::string j = "{\"id\":" + std::to_string(id) +
                    ",\"sharing\":" + std::to_string(sharing) +
                    ",\"ac\":" + (ac ? "true" : "false") +
                    ",\"rent\":" + std::to_string(rent) +
                    ",\"capacity\":" + std::to_string(getCapacity()) +
                    ",\"availableBeds\":" + std::to_string(availableBeds()) +
                    ",\"beds\":[";
    for (size_t i = 0; i < beds.size(); i++) {
        if (i) j += ",";
        j += beds[i].toJson();
    }
    return j + "]}";
}
