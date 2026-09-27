// RecommendationEngine.h - rule-based filtering + scoring + sorting (no ML)
#pragma once
#include <string>
#include <vector>
#include "Hostel.h"
#include "Student.h"

// Food type names used in hostels
const std::string FOOD_VEG  = "Veg";
const std::string FOOD_BOTH = "Veg + Non-Veg";

struct Recommendation {
    const Hostel* hostel;     // the suitable hostel
    const Room* bestRoom;     // cheapest room that matches the student
    double distance;          // km from the student's college
    int score;                // 0 - 100
    int matchingRooms;        // rooms that match the student's preferences
    int matchingBeds;         // free beds in those rooms

    std::string toJson() const;
};

class RecommendationEngine {
public:
    // Step 1: filter   Step 2: score   Step 3: sort (nearest first)
    std::vector<Recommendation> recommend(const Student& student,
                                          const std::vector<Hostel>& hostels) const;

private:
    bool hostelMatches(const Student& s, const Hostel& h, double& distance) const;
    bool roomMatches(const Student& s, const Room& r) const;
    int calculateScore(const Student& s, const Hostel& h, const Room& r, double distance) const;
};
