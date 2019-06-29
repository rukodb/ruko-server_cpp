#include "FloatData.hpp"

Str FloatData::toString() const {
    char buf[64];
    sprintf(buf, "%-.*g", 8, val);
    return buf;
}
