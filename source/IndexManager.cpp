#include "IndexManager.hpp"

Vec<Str> split(const Str &s, char delim) {
    Vec<Str> result;
    std::stringstream ss(s);
    Str item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}
