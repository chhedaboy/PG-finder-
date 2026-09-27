#include "Person.h"

Person::Person(int id, const std::string& name) : id(id), name(name) {}

int Person::getId() const { return id; }
const std::string& Person::getName() const { return name; }
