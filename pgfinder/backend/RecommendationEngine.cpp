#include "RecommendationEngine.h"
#include <algorithm>
#include "Json.h"

// ---- Filtering ---------------------------------------------------------

// Hostel-level checks: college, gender, distance, food
bool RecommendationEngine::hostelMatches(const Student& s, const Hostel& h, double& distance) const {
    // college: hostel must have a distance stored for the student's college
    if (!h.distanceTo(s.getCollege(), distance)) return false;

    // gender: Male student -> Male hostel only, Female student -> Female hostel only
    if (h.getGender() != s.getGender()) return false;

    // maximum acceptable distance
    if (distance > s.getMaxDistance()) return false;

    // food: a Non-Veg student needs a hostel that serves Non-Veg.
    // (Veg food is available in every hostel, so a Veg student can go anywhere.)
    if (s.getFoodPref() == "Non-Veg" && h.getFood() != FOOD_BOTH) return false;

    return true;
}

// Room-level checks: availability, budget, sharing, AC
bool RecommendationEngine::roomMatches(const Student& s, const Room& r) const {
    if (r.availableBeds() <= 0) return false;                                   // availability
    if (r.getRent() > s.getBudget()) return false;                              // budget
    if (s.getSharingPref() != 0 && r.getSharing() != s.getSharingPref()) return false;  // sharing
    if (s.getAcPref() == "AC" && !r.isAC()) return false;                       // AC
    if (s.getAcPref() == "Non-AC" && r.isAC()) return false;
    return true;
}

// ---- Scoring (total 100) ------------------------------------------------
//  Gender 10 + Distance 30 + Budget 25 + Food 15 + Sharing 10 + AC 10
int RecommendationEngine::calculateScore(const Student& s, const Hostel& h, const Room& r,
                                         double distance) const {
    double score = 0;

    score += 10;   // gender (hostels of the wrong gender are already filtered out)

    // distance: 30 points when very close, 10 points when exactly at the limit
    double dRatio = distance / s.getMaxDistance();
    score += 10 + 20 * (1 - dRatio);

    // budget: 25 points when very cheap, 15 points when exactly at the budget
    double bRatio = (double)r.getRent() / s.getBudget();
    score += 15 + 10 * (1 - bRatio);

    // food: full points for an exact match, fewer for a Veg student in a mixed hostel
    if (s.getFoodPref() == "Veg" && h.getFood() == FOOD_BOTH) score += 10;
    else score += 15;

    score += 10;   // sharing (already matched, or student had no preference)
    score += 10;   // AC      (already matched, or student had no preference)

    if (score > 100) score = 100;
    if (score < 0) score = 0;
    return (int)(score + 0.5);
}

// ---- Main function ------------------------------------------------------
std::vector<Recommendation> RecommendationEngine::recommend(const Student& student,
                                                            const std::vector<Hostel>& hostels) const {
    std::vector<Recommendation> result;

    for (const Hostel& h : hostels) {
        double distance = 0;
        if (!hostelMatches(student, h, distance)) continue;

        // find all rooms that match; remember the cheapest one as the "best room"
        const Room* best = nullptr;
        int rooms = 0, beds = 0;
        for (const Room& r : h.getRooms()) {
            if (!roomMatches(student, r)) continue;
            rooms++;
            beds += r.availableBeds();
            if (best == nullptr || r.getRent() < best->getRent()) best = &r;
        }
        if (best == nullptr) continue;   // no suitable room in this hostel

        Recommendation rec;
        rec.hostel = &h;
        rec.bestRoom = best;
        rec.distance = distance;
        rec.score = calculateScore(student, h, *best, distance);
        rec.matchingRooms = rooms;
        rec.matchingBeds = beds;
        result.push_back(rec);
    }

    // sort: nearest first; if two hostels are at the same distance, higher score first
    std::sort(result.begin(), result.end(), [](const Recommendation& a, const Recommendation& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        return a.score > b.score;
    });

    return result;
}

std::string Recommendation::toJson() const {
    return "{\"hostelId\":" + std::to_string(hostel->getId()) +
           ",\"name\":" + jsonStr(hostel->getName()) +
           ",\"location\":" + jsonStr(hostel->getLocation()) +
           ",\"gender\":" + jsonStr(hostel->getGender()) +
           ",\"food\":" + jsonStr(hostel->getFood()) +
           ",\"distance\":" + jsonNum(distance) +
           ",\"score\":" + std::to_string(score) +
           ",\"matchingRooms\":" + std::to_string(matchingRooms) +
           ",\"matchingBeds\":" + std::to_string(matchingBeds) +
           ",\"room\":{\"id\":" + std::to_string(bestRoom->getId()) +
           ",\"sharing\":" + std::to_string(bestRoom->getSharing()) +
           ",\"ac\":" + (bestRoom->isAC() ? "true" : "false") +
           ",\"rent\":" + std::to_string(bestRoom->getRent()) +
           ",\"availableBeds\":" + std::to_string(bestRoom->availableBeds()) + "}}";
}
