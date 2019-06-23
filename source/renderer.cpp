#include "renderer.hpp"


Str jsonStr(const void *) {
    return "null";
}

Str jsonStr(int x) {
    return std::to_string(x);
}

Str jsonStr(float x) {
    return std::to_string(x);
}

Str jsonStr(bool x) {
    return x ? "true" : "false";
}

Str jsonStr(const Object::Ptr &o) {
    return o ? o->toString() : "?";
}

Str jsonStr(const Str &str) {
    return "\"" + str + "\"";
}

Str jsonStr(const Vec<Object::Ptr> &array) {
    if (array.empty()) {
        return "[]";
    }
    Vec<Str> parts;
    parts.emplace_back("[");
    for (auto &i : array) {
        parts.emplace_back(jsonStr(i));
        parts.emplace_back(", ");
    }
    parts.pop_back();
    parts.emplace_back("]");
    return concat(parts);
}

Str jsonStr(const Map<Str, Object::Ptr> &map) {
    if (map.empty()) {
        return "{}";
    }
    Vec<Str> parts;
    parts.emplace_back("{");
    for (auto &i : map) {
        parts.emplace_back(jsonStr(i.first));
        parts.emplace_back(": ");
        parts.emplace_back(jsonStr(i.second));
        parts.emplace_back(", ");
    }
    parts.pop_back();
    parts.emplace_back("}");
    return concat(parts);
}
