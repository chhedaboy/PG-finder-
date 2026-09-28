#include "Booking.h"
#include "Json.h"

Booking::Booking(int id, int studentId, const std::string& studentName, const std::string& college,
                 int hostelId, const std::string& hostelName, int roomId, int bedId, int rent,
         const std::string& time, const std::string& gender, const std::string& contact)
    : id(id), studentId(studentId), studentName(studentName), college(college),
      hostelId(hostelId), hostelName(hostelName), roomId(roomId), bedId(bedId), rent(rent), time(time),
      gender(gender), contact(contact) {}

int Booking::getId() const { return id; }
int Booking::getHostelId() const { return hostelId; }

// bookings.txt appends gender and contact after the original fields for compatibility.
std::string Booking::toRecord() const {
    return std::to_string(id) + "|" + std::to_string(studentId) + "|" + studentName + "|" + college + "|" +
           std::to_string(hostelId) + "|" + hostelName + "|" + std::to_string(roomId) + "|" +
           std::to_string(bedId) + "|" + std::to_string(rent) + "|" + time + "|" + gender + "|" + contact;
}

std::string Booking::toJson() const {
    return "{\"id\":" + std::to_string(id) +
           ",\"studentName\":" + jsonStr(studentName) +
           ",\"gender\":" + jsonStr(gender) +
           ",\"contact\":" + jsonStr(contact) +
           ",\"college\":" + jsonStr(college) +
           ",\"hostelId\":" + std::to_string(hostelId) +
           ",\"hostelName\":" + jsonStr(hostelName) +
           ",\"roomId\":" + std::to_string(roomId) +
           ",\"bedId\":" + std::to_string(bedId) +
           ",\"rent\":" + std::to_string(rent) +
           ",\"time\":" + jsonStr(time) + "}";
}
