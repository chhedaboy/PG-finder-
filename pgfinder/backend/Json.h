// Json.h - tiny helpers to build JSON text (responses are built in C++)
#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>

inline std::string jsonEscape(const std::string& s) {
    std::string o;
    for (unsigned char c : s) {
        switch (c) {
            case '"':  o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n";  break;
            case '\r': o += "\\r";  break;
            case '\t': o += "\\t";  break;
            default:
                if (c < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", c); o += b; }
                else o += (char)c;
        }
    }
    return o;
}

inline std::string jsonStr(const std::string& s) { return "\"" + jsonEscape(s) + "\""; }

inline std::string jsonNum(double v) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << v;
    return ss.str();
}

inline std::string jsonError(const std::string& msg) {
    return "{\"ok\":false,\"error\":" + jsonStr(msg) + "}";
}
