// Owner.h - a hostel owner
#pragma once
#include "Person.h"

class Owner : public Person {
public:
    Owner(int id, const std::string& name);

    std::string getRole() const override;
    std::string toRecord() const override;
    std::string toJson() const;
};
