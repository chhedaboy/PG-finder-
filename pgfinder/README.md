# PG Finder Pune - C++ OOP Project

A website where students find and book a PG/Hostel near their college in Pune, and owners manage hostels.
**All logic is in C++** (recommendation, filtering, scoring, sorting, booking, file handling, HTTP server).
The website (HTML/CSS/JS) only shows pages and calls the C++ server.

## Run it

You only need a C++ compiler (g++ 7 or newer). Open a terminal in this folder (the one containing `backend/` and `frontend/`).

**Linux / macOS**
```
g++ -std=c++17 backend/*.cpp -o server
./server
```

**Windows (MinGW-w64 / MSYS2 g++)**
```
g++ -std=c++17 backend/*.cpp -o server.exe -lws2_32
server.exe
```

**Or with CMake (any OS)**
```
cmake -S . -B build
cmake --build build
./build/server        (Windows: build\Debug\server.exe or build\server.exe)
```
(Run it from this project folder so it can find `backend/data` and `frontend`.)

Then open **http://localhost:8080** in your browser. Use another port with `./server 9000`.

## How to use

- **Student:** Find a Hostel -> fill the form -> Search -> View & Book -> choose sharing type -> tap a green bed -> Confirm with a phone number or email address.
- **Owner:** Hostel Owner -> log in with a name (sample owner: **Rajesh Patil**; a new name registers a new owner) -> view confirmed booking details and student contact info, or add / manage / remove hostels and rooms.

Sample data: 36 hostels, 6 colleges, 8 rooms per hostel. Hostels near Bharati Vidyapeeth, PICT and VIT are shared
(same hostel, different distance per college). All names and numbers are made-up project data.
To reset all bookings/changes, restore the files in `backend/data/` from your original copy.

## Project structure

```
backend/
  main.cpp                 server start + API routes
  HttpServer.h/.cpp        tiny HTTP server (sockets, no library)
  Person.h/.cpp            abstract base class  -> Student, Owner
  Student.h/.cpp, Owner.h/.cpp
  Accommodation.h/.cpp     abstract base class  -> Hostel
  Hostel.h/.cpp            hostel (distance per college, list of rooms)
  Room.h/.cpp, Bed.h/.cpp  room (1/2/3 sharing) and its beds
  Booking.h/.cpp           booking confirmation
  RecommendationEngine.h/.cpp   filter + score + sort
  SystemManager.h/.cpp     holds all objects (vectors) + reads/writes the text files
  Colleges.h, Json.h, Util.h    small helpers
  data/                    hostels.txt rooms.txt students.txt owners.txt bookings.txt
frontend/                  index.html, style.css, app.js
```

## OOP concepts (for viva)

- **Classes & objects:** Student, Owner, Hostel, Room, Bed, Booking, RecommendationEngine, SystemManager.
- **Encapsulation:** data members are `private`/`protected`; accessed through getters/setters and methods like `Room::bookBed()`.
- **Abstraction:** `Accommodation` and `Person` are abstract (pure virtual functions).
- **Inheritance:** `Hostel` extends `Accommodation`; `Student` and `Owner` extend `Person`.
- **Polymorphism:** `getRole()`, `toRecord()`, `checkAvailability()` are virtual and overridden in the child classes.
- **STL:** `vector` (hostels, rooms, beds), `map` (distance per college), `sort` (nearest first), `string`.
- **File handling:** `ifstream` / `ofstream` in `SystemManager` (pipe-separated text files).

## Recommendation logic (simple rules)

1. **Filter hostel:** college distance exists, gender matches, distance <= maximum, food OK
   (Non-Veg needs a "Veg + Non-Veg" hostel; Veg students can stay anywhere).
2. **Filter rooms:** has a free bed, rent <= budget, sharing matches, AC matches (if chosen).
3. **Score (out of 100):** Gender 10 + Distance 30 + Budget 25 + Food 15 + Sharing 10 + AC 10.
4. **Sort:** nearest hostel first (ties: higher score first).

## API (used by the website)

| Route | Purpose |
|---|---|
| GET `/api/colleges` | list of colleges |
| GET `/api/search` | recommendations (name, college, gender, budget, maxDist, food, sharing, ac) |
| GET `/api/hostel` | hostel details with rooms and beds |
| POST `/api/book` | book a bed with student contact details |
| POST `/api/owner/login` | owner login / register |
| GET `/api/owner/hostels` | owner's hostels |
| POST `/api/owner/hostel/add`, `/update`, `/remove` | manage hostels |
| POST `/api/owner/room/add`, `/update`, `/remove` | manage rooms and beds |

Owner login is name-only (no passwords) to keep the project simple. Rent must be between 6000 and 20000.
