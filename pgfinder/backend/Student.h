// Student.h - a student and the preferences used for searching
#pragma once
#include "Person.h"

class Student : public Person {
private:
    std::string gender;      // "Male" or "Female"
    std::string college;     // college id, e.g. "PICT"
    double budget;           // max monthly rent
    double maxDistance;      // max km from college
    std::string foodPref;    // "Veg", "Non-Veg", "Any"
    int sharingPref;         // 1, 2, 3 or 0 (= no preference)
    std::string acPref;      // "AC", "Non-AC", "Any"

public:
    Student(int id, const std::string& name, const std::string& gender, const std::string& college);

    void setPreferences(double budget, double maxDistance, const std::string& foodPref,
                        int sharingPref, const std::string& acPref);

    const std::string& getGender() const;
    const std::string& getCollege() const;
    double getBudget() const;
    double getMaxDistance() const;
    const std::string& getFoodPref() const;
    int getSharingPref() const;
    const std::string& getAcPref() const;

    std::string getRole() const override;
    std::string toRecord() const override;
};
