#include "Accommodation.h"

Accommodation::Accommodation(const std::string& name, const std::string& location,
                             const std::string& gender, const std::string& food)
    : name(name), location(location), gender(gender), food(food) {}

const std::string& Accommodation::getName() const { return name; }
const std::string& Accommodation::getLocation() const { return location; }
const std::string& Accommodation::getGender() const { return gender; }
const std::string& Accommodation::getFood() const { return food; }

void Accommodation::setName(const std::string& v) { name = v; }
void Accommodation::setLocation(const std::string& v) { location = v; }
void Accommodation::setGender(const std::string& v) { gender = v; }
void Accommodation::setFood(const std::string& v) { food = v; }
