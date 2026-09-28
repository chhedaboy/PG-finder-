#include "SystemManager.h"
#include <fstream>
#include "Util.h"

SystemManager::SystemManager(const std::string& dataDir)
    : dataDir(dataDir), nextHostelId(1), nextOwnerId(1), nextStudentId(1), nextBookingId(1) {}

// ---- file helpers -------------------------------------------------------

std::vector<std::string> SystemManager::readLines(const std::string& file) const {
    std::vector<std::string> lines;
    std::ifstream in(dataDir + "/" + file);
    std::string line;
    while (std::getline(in, line)) {
        line = trimStr(line);
        if (!line.empty() && line[0] != '#') lines.push_back(line);
    }
    return lines;
}

void SystemManager::writeLines(const std::string& file, const std::vector<std::string>& lines) const {
    std::ofstream out(dataDir + "/" + file);
    for (const std::string& l : lines) out << l << "\n";
}

// ---- load / save --------------------------------------------------------

void SystemManager::load() {
    hostels.clear(); owners.clear(); students.clear(); bookings.clear();

    // owners.txt: id|name
    for (const std::string& line : readLines("owners.txt")) {
        std::vector<std::string> f = splitStr(line, '|');
        int id;
        if (f.size() < 2 || !parseInt(f[0], id)) continue;
        owners.push_back(Owner(id, f[1]));
        if (id >= nextOwnerId) nextOwnerId = id + 1;
    }

    // students.txt: id|name|gender|college
    for (const std::string& line : readLines("students.txt")) {
        std::vector<std::string> f = splitStr(line, '|');
        int id;
        if (f.size() < 4 || !parseInt(f[0], id)) continue;
        students.push_back(Student(id, f[1], f[2], f[3]));
        if (id >= nextStudentId) nextStudentId = id + 1;
    }

    // hostels.txt: id|ownerId|name|location|gender|food|COEP=4.2;DYP=7.8;...
    for (const std::string& line : readLines("hostels.txt")) {
        std::vector<std::string> f = splitStr(line, '|');
        int id, ownerId;
        if (f.size() < 7 || !parseInt(f[0], id) || !parseInt(f[1], ownerId)) continue;
        Hostel h(id, ownerId, f[2], f[3], f[4], f[5]);
        for (const std::string& pair : splitStr(f[6], ';')) {
            std::vector<std::string> kv = splitStr(pair, '=');
            double km;
            if (kv.size() == 2 && parseDouble(kv[1], km)) h.setDistance(kv[0], km);
        }
        hostels.push_back(h);
        if (id >= nextHostelId) nextHostelId = id + 1;
    }

    // rooms.txt: hostelId|roomId|sharing|ac|rent|bedStates
    for (const std::string& line : readLines("rooms.txt")) {
        std::vector<std::string> f = splitStr(line, '|');
        int hostelId, roomId, sharing, ac, rent;
        if (f.size() < 6 || !parseInt(f[0], hostelId) || !parseInt(f[1], roomId) ||
            !parseInt(f[2], sharing) || !parseInt(f[3], ac) || !parseInt(f[4], rent)) continue;
        Hostel* h = findHostel(hostelId);
        if (h != nullptr && sharing >= 1 && sharing <= 3)
            h->addRoom(Room(roomId, sharing, ac == 1, rent, f[5]));
    }

    // Older booking rows may not have the appended gender and contact fields.
    for (const std::string& line : readLines("bookings.txt")) {
        std::vector<std::string> f = splitStr(line, '|');
        int id, sid, hid, rid, bid, rent;
        if (f.size() < 10 || !parseInt(f[0], id) || !parseInt(f[1], sid) || !parseInt(f[4], hid) ||
            !parseInt(f[6], rid) || !parseInt(f[7], bid) || !parseInt(f[8], rent)) continue;
        const std::string gender = f.size() > 10 ? f[10] : "";
        const std::string contact = f.size() > 11 ? f[11] : "";
        bookings.push_back(Booking(id, sid, f[2], f[3], hid, f[5], rid, bid, rent, f[9], gender, contact));
        if (id >= nextBookingId) nextBookingId = id + 1;
    }
}

void SystemManager::saveAll() {
    std::vector<std::string> lines;

    for (const Owner& o : owners) lines.push_back(o.toRecord());
    writeLines("owners.txt", lines);

    lines.clear();
    for (const Student& s : students) lines.push_back(s.toRecord());
    writeLines("students.txt", lines);

    lines.clear();
    for (const Hostel& h : hostels) lines.push_back(h.toRecord());
    writeLines("hostels.txt", lines);

    lines.clear();
    for (const Hostel& h : hostels)
        for (const Room& r : h.getRooms()) lines.push_back(r.toRecord(h.getId()));
    writeLines("rooms.txt", lines);

    lines.clear();
    for (const Booking& b : bookings) lines.push_back(b.toRecord());
    writeLines("bookings.txt", lines);
}

// ---- hostels ------------------------------------------------------------

const std::vector<Hostel>& SystemManager::getHostels() const { return hostels; }
const std::vector<Booking>& SystemManager::getBookings() const { return bookings; }

Hostel* SystemManager::findHostel(int id) {
    for (Hostel& h : hostels)
        if (h.getId() == id) return &h;
    return nullptr;
}

int SystemManager::addHostel(int ownerId, const std::string& name, const std::string& location,
                             const std::string& gender, const std::string& food,
                             const std::map<std::string, double>& distances) {
    Hostel h(nextHostelId++, ownerId, name, location, gender, food);
    for (const auto& kv : distances) h.setDistance(kv.first, kv.second);
    hostels.push_back(h);
    saveAll();
    return h.getId();
}

bool SystemManager::removeHostel(int hostelId) {
    for (size_t i = 0; i < hostels.size(); i++) {
        if (hostels[i].getId() == hostelId) {
            hostels.erase(hostels.begin() + i);
            saveAll();
            return true;
        }
    }
    return false;
}

// ---- owners / students ----------------------------------------------------

Owner* SystemManager::findOwner(int id) {
    for (Owner& o : owners)
        if (o.getId() == id) return &o;
    return nullptr;
}

Owner& SystemManager::loginOwner(const std::string& name) {
    for (Owner& o : owners)
        if (toLowerStr(o.getName()) == toLowerStr(name)) return o;
    owners.push_back(Owner(nextOwnerId++, name));
    saveAll();
    return owners.back();
}

Student& SystemManager::registerStudent(const std::string& name, const std::string& gender,
                                        const std::string& college) {
    for (Student& s : students)
        if (toLowerStr(s.getName()) == toLowerStr(name) && s.getGender() == gender &&
            s.getCollege() == college) return s;
    students.push_back(Student(nextStudentId++, name, gender, college));
    return students.back();
}

// ---- booking --------------------------------------------------------------

bool SystemManager::bookBed(int hostelId, int roomId, int bedId, const std::string& studentName,
                            const std::string& gender, const std::string& college,
                            const std::string& contact,
                            Booking& confirmation, std::string& error) {
    Hostel* h = findHostel(hostelId);
    if (h == nullptr) { error = "Hostel not found (it may have been removed)."; return false; }
    if (h->getGender() != gender) { error = "This hostel is for " + h->getGender() + " students only."; return false; }

    Room* r = h->findRoom(roomId);
    if (r == nullptr) { error = "Room not found."; return false; }

    if (!r->bookBed(bedId)) { error = "Sorry, this bed is not available anymore."; return false; }

    Student& s = registerStudent(studentName, gender, college);
    Booking b(nextBookingId++, s.getId(), studentName, college, hostelId, h->getName(),
              roomId, bedId, r->getRent(), currentTimeString(), gender, contact);
    bookings.push_back(b);
    saveAll();

    confirmation = b;
    return true;
}
