#include "StringData.hpp"
#include <sstream>


Str StringData::escaped(const Str &s) {
    std::stringstream ss;
    const char *cstr = s.c_str();
    for (const char *c = cstr; c != cstr + s.size(); ++c) {
        switch (*c) {
            case '\\':
                ss << "\\\\";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '"':
                ss << "\\\"";
                break;
            default:
                ss << *c;
                break;
        }
    }
    return ss.str();
}

Str StringData::toString() const {
    return '"' + escaped(val) + '"';
}
