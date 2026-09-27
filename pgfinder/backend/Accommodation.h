// Accommodation.h - abstract base class (abstraction + inheritance)
#pragma once
#include <string>

class Accommodation {
protected:
    std::string name;
    std::string location;
    std::string gender;   // "Male" or "Female"
    std::string food;     // "Veg" or "Veg + Non-Veg"

public:
    Accommodation(const std::string& name, const std::string& location,
                  const std::string& gender, const std::string& food);
    virtual ~Accommodation() = default;

    const std::string& getName() const;
    const std::string& getLocation() const;
    const std::string& getGender() const;
    const std::string& getFood() const;

    void setName(const std::string& v);
    void setLocation(const std::string& v);
    void setGender(const std::string& v);
    void setFood(const std::string& v);

    // Every kind of accommodation must say whether it has a free bed.
    virtual bool checkAvailability() const = 0;
    virtual std::string getType() const = 0;
};
