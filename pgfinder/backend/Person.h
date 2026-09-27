// Person.h - abstract base class for Student and Owner
#pragma once
#include <string>

class Person {
protected:
    int id;
    std::string name;

public:
    Person(int id, const std::string& name);
    virtual ~Person() = default;

    int getId() const;
    const std::string& getName() const;

    virtual std::string getRole() const = 0;     // polymorphism: each subclass answers differently
    virtual std::string toRecord() const = 0;    // one line for the text file
};
