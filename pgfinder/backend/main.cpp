// main.cpp - starts the server and routes each request to the right C++ class.
// The website (HTML/CSS/JS) only calls these routes; all logic is done here in C++.
#include <cctype>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Colleges.h"
#include "HttpServer.h"
#include "Json.h"
#include "RecommendationEngine.h"
#include "SystemManager.h"
#include "Util.h"

static SystemManager* manager = nullptr;
static RecommendationEngine engine;
static std::string frontendDir;

// ---- small helpers -----------------------------------------------------------

static std::string param(const HttpRequest& req, const std::string& key) {
    auto it = req.params.find(key);
    return it == req.params.end() ? "" : trimStr(it->second);
}

static HttpResponse jsonReply(const std::string& body, int status = 200) {
    HttpResponse r;
    r.status = status;
    r.body = body;
    return r;
}

static HttpResponse errorReply(const std::string& message) { return jsonReply(jsonError(message), 400); }

static bool fileExists(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

static bool validGender(const std::string& g) { return g == "Male" || g == "Female"; }

static bool validContact(const std::string& contact) {
    size_t at = contact.find('@');
    if (at != std::string::npos) {
        size_t dot = contact.find('.', at + 1);
        return at > 0 && contact.find('@', at + 1) == std::string::npos &&
               dot != std::string::npos && dot + 1 < contact.size() &&
               contact.find_first_of(" \t\r\n") == std::string::npos;
    }

    int digits = 0;
    for (unsigned char c : contact) {
        if (std::isdigit(c)) digits++;
        else if (c != '+' && c != '-' && c != '(' && c != ')' && c != ' ') return false;
    }
    return digits >= 7 && digits <= 15;
}

// ---- student side ---------------------------------------------------------------

static HttpResponse handleColleges() {
    std::string j = "{\"ok\":true,\"colleges\":[";
    const std::vector<College>& list = allColleges();
    for (size_t i = 0; i < list.size(); i++) {
        if (i) j += ",";
        j += "{\"id\":" + jsonStr(list[i].id) + ",\"name\":" + jsonStr(list[i].name) + "}";
    }
    return jsonReply(j + "]}");
}

// GET /api/search?name=&college=&gender=&budget=&maxDist=&food=&sharing=&ac=
static HttpResponse handleSearch(const HttpRequest& req) {
    std::string name = cleanField(param(req, "name"));
    std::string collegeId = param(req, "college");
    std::string gender = param(req, "gender");
    std::string food = param(req, "food");
    std::string ac = param(req, "ac");
    double budget = 0, maxDist = 0;
    int sharing = 0;

    if (name.empty()) return errorReply("Please enter your name.");
    const College* college = findCollege(collegeId);
    if (college == nullptr) return errorReply("Please select a college.");
    if (!validGender(gender)) return errorReply("Please select your gender.");
    if (!parseDouble(param(req, "budget"), budget) || budget <= 0) return errorReply("Please enter a valid budget.");
    if (!parseDouble(param(req, "maxDist"), maxDist) || maxDist <= 0) return errorReply("Please enter a valid maximum distance.");
    if (food != "Veg" && food != "Non-Veg" && food != "Any") return errorReply("Please select a food preference.");
    if (!parseInt(param(req, "sharing"), sharing) || sharing < 0 || sharing > 3) return errorReply("Please select a sharing preference.");
    if (ac.empty()) ac = "Any";
    if (ac != "AC" && ac != "Non-AC" && ac != "Any") return errorReply("Invalid AC preference.");

    Student student(0, name, gender, collegeId);
    student.setPreferences(budget, maxDist, food, sharing, ac);

    std::vector<Recommendation> results = engine.recommend(student, manager->getHostels());

    std::string j = "{\"ok\":true,\"college\":" + jsonStr(college->name) +
                    ",\"count\":" + std::to_string(results.size()) + ",\"results\":[";
    for (size_t i = 0; i < results.size(); i++) {
        if (i) j += ",";
        j += results[i].toJson();
    }
    return jsonReply(j + "]}");
}

// GET /api/hostel?id=&college=
static HttpResponse handleHostel(const HttpRequest& req) {
    int id;
    if (!parseInt(param(req, "id"), id)) return errorReply("Invalid hostel id.");
    Hostel* h = manager->findHostel(id);
    if (h == nullptr) return errorReply("Hostel not found (it may have been removed).");
    return jsonReply("{\"ok\":true,\"hostel\":" + h->toJson(param(req, "college")) + "}");
}

// POST /api/book  (hostelId, roomId, bedId, name, gender, college)
static HttpResponse handleBook(const HttpRequest& req) {
    int hostelId, roomId, bedId;
    if (!parseInt(param(req, "hostelId"), hostelId) || !parseInt(param(req, "roomId"), roomId) ||
        !parseInt(param(req, "bedId"), bedId)) return errorReply("Please select a bed.");

    std::string name = cleanField(param(req, "name"));
    std::string gender = param(req, "gender");
    std::string college = param(req, "college");
    std::string contact = cleanField(param(req, "contact"));
    if (name.empty()) return errorReply("Please enter your name.");
    if (!validGender(gender)) return errorReply("Please select your gender.");
    if (findCollege(college) == nullptr) return errorReply("Please select your college.");
    if (!validContact(contact)) return errorReply("Please enter a valid phone number or email address.");

    Booking confirmation(0, 0, "", "", 0, "", 0, 0, 0, "");
    std::string error;
    if (!manager->bookBed(hostelId, roomId, bedId, name, gender, college, contact, confirmation, error))
        return errorReply(error);
    return jsonReply("{\"ok\":true,\"booking\":" + confirmation.toJson() + "}");
}

// ---- owner side -----------------------------------------------------------------

// POST /api/owner/login  (name)  -> finds the owner, or creates a new one
static HttpResponse handleOwnerLogin(const HttpRequest& req) {
    std::string name = cleanField(param(req, "name"));
    if (name.empty()) return errorReply("Please enter your name.");
    Owner& o = manager->loginOwner(name);
    return jsonReply("{\"ok\":true,\"owner\":" + o.toJson() + "}");
}

static Owner* requireOwner(const HttpRequest& req, std::string& error) {
    int id;
    Owner* o = parseInt(param(req, "ownerId"), id) ? manager->findOwner(id) : nullptr;
    if (o == nullptr) error = "Please log in as an owner first.";
    return o;
}

// Finds the hostel and checks that it belongs to the logged-in owner
static Hostel* requireOwnedHostel(const HttpRequest& req, const Owner& owner, std::string& error) {
    int id;
    Hostel* h = parseInt(param(req, "hostelId"), id) ? manager->findHostel(id) : nullptr;
    if (h == nullptr) { error = "Hostel not found."; return nullptr; }
    if (h->getOwnerId() != owner.getId()) { error = "You can only manage your own hostels."; return nullptr; }
    return h;
}

// GET /api/owner/hostels?ownerId=
static HttpResponse handleOwnerHostels(const HttpRequest& req) {
    std::string error;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);

    std::string j = "{\"ok\":true,\"hostels\":[";
    bool first = true;
    for (const Hostel& h : manager->getHostels()) {
        if (h.getOwnerId() != owner->getId()) continue;
        if (!first) j += ",";
        std::string hostelJson = h.toJson();
        hostelJson.erase(hostelJson.size() - 1);
        hostelJson += ",\"bookings\":[";
        bool firstBooking = true;
        for (const Booking& booking : manager->getBookings()) {
            if (booking.getHostelId() != h.getId()) continue;
            if (!firstBooking) hostelJson += ",";
            hostelJson += booking.toJson();
            firstBooking = false;
        }
        hostelJson += "]}";
        j += hostelJson;
        first = false;
    }
    return jsonReply(j + "]}");
}

// Reads and checks hostel details plus its nearest college and distance.
static bool readHostelFields(const HttpRequest& req, std::string& name, std::string& location,
                             std::string& gender, std::string& food,
                             std::map<std::string, double>& distances, std::string& error) {
    name = cleanField(param(req, "name"));
    location = cleanField(param(req, "location"));
    gender = param(req, "gender");
    food = param(req, "food");

    if (name.empty() || location.empty()) { error = "Hostel name and location are required."; return false; }
    if (!validGender(gender)) { error = "Please select Male or Female."; return false; }
    if (food != FOOD_VEG && food != FOOD_BOTH) { error = "Please select the food type."; return false; }

    const std::string collegeId = param(req, "nearCollege");
    const College* college = findCollege(collegeId);
    double km;
    if (college == nullptr) { error = "Please select the nearest college."; return false; }
    if (!parseDouble(param(req, "nearDistance"), km) || !std::isfinite(km) || km <= 0 || km > 100) {
        error = "Please enter a valid distance (km) from " + college->name + ".";
        return false;
    }
    distances[collegeId] = km;
    return true;
}

// POST /api/owner/hostel/add
static HttpResponse handleAddHostel(const HttpRequest& req) {
    std::string error, name, location, gender, food;
    std::map<std::string, double> distances;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    if (!readHostelFields(req, name, location, gender, food, distances, error)) return errorReply(error);

    int id = manager->addHostel(owner->getId(), name, location, gender, food, distances);
    return jsonReply("{\"ok\":true,\"hostelId\":" + std::to_string(id) + "}");
}

// POST /api/owner/hostel/update
static HttpResponse handleUpdateHostel(const HttpRequest& req) {
    std::string error, name, location, gender, food;
    std::map<std::string, double> distances;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    Hostel* h = requireOwnedHostel(req, *owner, error);
    if (h == nullptr) return errorReply(error);
    if (!readHostelFields(req, name, location, gender, food, distances, error)) return errorReply(error);

    h->setName(name);
    h->setLocation(location);
    h->setGender(gender);
    h->setFood(food);
    h->clearDistances();
    for (const auto& kv : distances) h->setDistance(kv.first, kv.second);
    manager->saveAll();
    return jsonReply("{\"ok\":true}");
}

// POST /api/owner/hostel/remove
static HttpResponse handleRemoveHostel(const HttpRequest& req) {
    std::string error;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    Hostel* h = requireOwnedHostel(req, *owner, error);
    if (h == nullptr) return errorReply(error);

    manager->removeHostel(h->getId());
    return jsonReply("{\"ok\":true}");
}

static bool validRent(int rent) { return rent >= 6000 && rent <= 20000; }

// POST /api/owner/room/add  (sharing, ac, rent, count)
static HttpResponse handleAddRoom(const HttpRequest& req) {
    std::string error;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    Hostel* h = requireOwnedHostel(req, *owner, error);
    if (h == nullptr) return errorReply(error);

    int sharing, rent, count = 1;
    if (!parseInt(param(req, "sharing"), sharing) || sharing < 1 || sharing > 3) return errorReply("Sharing must be 1, 2 or 3.");
    if (!parseInt(param(req, "rent"), rent) || !validRent(rent)) return errorReply("Rent must be between 6000 and 20000.");
    if (!param(req, "count").empty() && (!parseInt(param(req, "count"), count) || count < 1 || count > 10))
        return errorReply("You can add 1 to 10 rooms at a time.");
    bool ac = param(req, "ac") == "1";

    for (int i = 0; i < count; i++) h->addRoom(Room(h->nextRoomId(), sharing, ac, rent));
    manager->saveAll();
    return jsonReply("{\"ok\":true}");
}

// POST /api/owner/room/update  (roomId, rent, ac, availableBeds)
static HttpResponse handleUpdateRoom(const HttpRequest& req) {
    std::string error;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    Hostel* h = requireOwnedHostel(req, *owner, error);
    if (h == nullptr) return errorReply(error);

    int roomId, rent, beds;
    Room* r = parseInt(param(req, "roomId"), roomId) ? h->findRoom(roomId) : nullptr;
    if (r == nullptr) return errorReply("Room not found.");
    if (!parseInt(param(req, "rent"), rent) || !validRent(rent)) return errorReply("Rent must be between 6000 and 20000.");
    if (!parseInt(param(req, "availableBeds"), beds) || beds < 0 || beds > r->getCapacity())
        return errorReply("Available beds must be between 0 and " + std::to_string(r->getCapacity()) + ".");

    r->setRent(rent);
    r->setAC(param(req, "ac") == "1");
    r->setAvailableBeds(beds);
    manager->saveAll();
    return jsonReply("{\"ok\":true}");
}

// POST /api/owner/room/remove  (roomId)
static HttpResponse handleRemoveRoom(const HttpRequest& req) {
    std::string error;
    Owner* owner = requireOwner(req, error);
    if (owner == nullptr) return errorReply(error);
    Hostel* h = requireOwnedHostel(req, *owner, error);
    if (h == nullptr) return errorReply(error);

    int roomId;
    if (!parseInt(param(req, "roomId"), roomId) || !h->removeRoom(roomId)) return errorReply("Room not found.");
    manager->saveAll();
    return jsonReply("{\"ok\":true}");
}

// ---- website files --------------------------------------------------------------

static std::string contentTypeFor(const std::string& path) {
    size_t dot = path.rfind('.');
    std::string ext = dot == std::string::npos ? "" : toLowerStr(path.substr(dot));
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "application/javascript; charset=utf-8";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".png")  return "image/png";
    return "application/octet-stream";
}

static HttpResponse serveFile(std::string path) {
    HttpResponse r;
    if (path == "/") path = "/index.html";
    if (path.find("..") != std::string::npos) { r.status = 403; r.contentType = "text/plain"; r.body = "Forbidden"; return r; }

    std::ifstream in((frontendDir + path).c_str(), std::ios::binary);
    if (!in) { r.status = 404; r.contentType = "text/plain"; r.body = "Not found"; return r; }

    std::ostringstream ss;
    ss << in.rdbuf();
    r.contentType = contentTypeFor(path);
    r.body = ss.str();
    return r;
}

// ---- router -------------------------------------------------------------------------

static HttpResponse handleRequest(const HttpRequest& req) {
    const std::string& p = req.path;

    if (p == "/api/colleges")            return handleColleges();
    if (p == "/api/search")              return handleSearch(req);
    if (p == "/api/hostel")              return handleHostel(req);
    if (p == "/api/book")                return handleBook(req);

    if (p == "/api/owner/login")         return handleOwnerLogin(req);
    if (p == "/api/owner/hostels")       return handleOwnerHostels(req);
    if (p == "/api/owner/hostel/add")    return handleAddHostel(req);
    if (p == "/api/owner/hostel/update") return handleUpdateHostel(req);
    if (p == "/api/owner/hostel/remove") return handleRemoveHostel(req);
    if (p == "/api/owner/room/add")      return handleAddRoom(req);
    if (p == "/api/owner/room/update")   return handleUpdateRoom(req);
    if (p == "/api/owner/room/remove")   return handleRemoveRoom(req);

    if (p.compare(0, 5, "/api/") == 0) return jsonReply(jsonError("Unknown API route."), 404);
    return serveFile(p);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) parseInt(argv[1], port);

    // works when started from the project folder OR from inside backend/
    std::string dataDir = fileExists("backend/data/hostels.txt") ? "backend/data" : "data";
    frontendDir = fileExists("frontend/index.html") ? "frontend" : "../frontend";

    if (!fileExists(dataDir + "/hostels.txt") || !fileExists(frontendDir + "/index.html")) {
        std::cout << "Could not find the data/ and frontend/ folders.\n"
                  << "Run the program from the project folder (the one that contains backend/ and frontend/).\n";
        return 1;
    }

    SystemManager sys(dataDir);
    sys.load();
    manager = &sys;

    std::cout << "PG Finder is running.\n"
              << "Open this in your browser:  http://localhost:" << port << "\n"
              << "(Press Ctrl+C to stop)\n";

    HttpServer server(port, handleRequest);
    if (!server.run()) {
        std::cout << "Could not start the server on port " << port
                  << ". Is another program using it? Try: server 9000\n";
        return 1;
    }
    return 0;
}
