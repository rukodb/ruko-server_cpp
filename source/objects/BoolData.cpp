#include "BoolData.hpp"

Str BoolData::toString() const {
    return val ? "true" : "false";
}
