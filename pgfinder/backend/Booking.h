// Booking.h - one confirmed bed booking (no payment, just a confirmation)
#pragma once
#include <string>

class Booking {
private:
    int id;
    int studentId;
    std::string studentName;
    std::string college;
    int hostelId;
    std::string hostelName;
    int roomId;
    int bedId;
    int rent;
    std::string time;
    std::string gender;
    std::string contact;

public:
    Booking(int id, int studentId, const std::string& studentName, const std::string& college,
            int hostelId, const std::string& hostelName, int roomId, int bedId, int rent,
            const std::string& time, const std::string& gender = "", const std::string& contact = "");

    int getId() const;
    int getHostelId() const;
    std::string toRecord() const;   // bookings.txt line
    std::string toJson() const;
};
