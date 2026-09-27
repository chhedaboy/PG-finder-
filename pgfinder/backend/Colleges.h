// Colleges.h - the 6 supported colleges (fixed list)
#pragma once
#include <string>
#include <vector>

struct College {
    std::string id;    // short code used in files and API
    std::string name;  // full name shown on the website
};

inline const std::vector<College>& allColleges() {
    static const std::vector<College> list = {
        {"COEP",   "College of Engineering, Pune (COEP)"},
        {"DYP",    "D.Y. Patil College of Engineering"},
        {"BVCOE",  "Bharati Vidyapeeth College of Engineering"},
        {"MITWPU", "MIT World Peace University (MIT-WPU)"},
        {"PICT",   "PICT"},
        {"VIT",    "VIT Pune"}
    };
    return list;
}

inline const College* findCollege(const std::string& id) {
    for (const College& c : allColleges())
        if (c.id == id) return &c;
    return nullptr;
}
