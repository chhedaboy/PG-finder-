#include "Student.h"

Student::Student(int id, const std::string& name, const std::string& gender, const std::string& college)
    : Person(id, name), gender(gender), college(college),
      budget(0), maxDistance(0), foodPref("Any"), sharingPref(0), acPref("Any") {}

void Student::setPreferences(double b, double d, const std::string& f, int s, const std::string& a) {
    budget = b; maxDistance = d; foodPref = f; sharingPref = s; acPref = a;
}

const std::string& Student::getGender() const { return gender; }
const std::string& Student::getCollege() const { return college; }
double Student::getBudget() const { return budget; }
double Student::getMaxDistance() const { return maxDistance; }
const std::string& Student::getFoodPref() const { return foodPref; }
int Student::getSharingPref() const { return sharingPref; }
const std::string& Student::getAcPref() const { return acPref; }

std::string Student::getRole() const { return "Student"; }

// students.txt line: id|name|gender|college
std::string Student::toRecord() const {
    return std::to_string(id) + "|" + name + "|" + gender + "|" + college;
}
