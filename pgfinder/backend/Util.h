// Util.h - small helper functions used by many classes
#pragma once
#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <algorithm>

inline std::vector<std::string> splitStr(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == sep) { out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}

inline std::string trimStr(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

inline std::string toLowerStr(std::string s) {
    for (char& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

// Removes characters that would break our text-file format.
inline std::string cleanField(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '|' || c == ';' || c == '=' || c == '\n' || c == '\r') o += ' ';
        else o += c;
    }
    return trimStr(o);
}

inline bool parseInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    if (*end != '\0') return false;
    out = (int)v;
    return true;
}

inline bool parseDouble(const std::string& s, double& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if (*end != '\0') return false;
    out = v;
    return true;
}

inline std::string currentTimeString() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&t));
    return buf;
}
