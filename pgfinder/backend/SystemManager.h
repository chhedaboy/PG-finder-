// SystemManager.h - keeps all objects in memory (STL vectors) and reads/writes the text files
#pragma once
#include <map>
#include <string>
#include <vector>
#include "Booking.h"
#include "Hostel.h"
#include "Owner.h"
#include "Student.h"

class SystemManager {
private:
    std::string dataDir;
    std::vector<Hostel> hostels;
    std::vector<Owner> owners;
    std::vector<Student> students;
    std::vector<Booking> bookings;
    int nextHostelId, nextOwnerId, nextStudentId, nextBookingId;

    std::vector<std::string> readLines(const std::string& file) const;
    void writeLines(const std::string& file, const std::vector<std::string>& lines) const;

public:
    explicit SystemManager(const std::string& dataDir);

    void load();       // read all files
    void saveAll();    // write all files

    // ---- hostels ----
    const std::vector<Hostel>& getHostels() const;
    Hostel* findHostel(int id);
    int addHostel(int ownerId, const std::string& name, const std::string& location,
                  const std::string& gender, const std::string& food,
                  const std::map<std::string, double>& distances);
    bool removeHostel(int hostelId);

    // ---- owners / students ----
    Owner* findOwner(int id);
    Owner& loginOwner(const std::string& name);    // finds the owner or registers a new one
    Student& registerStudent(const std::string& name, const std::string& gender,
                             const std::string& college);

    // ---- booking ----
    bool bookBed(int hostelId, int roomId, int bedId, const std::string& studentName,
                 const std::string& gender, const std::string& college,
                 Booking& confirmation, std::string& error);
};
