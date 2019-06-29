#include "NullData.hpp"

NullData::NullData(const Byte *data, size_t &p) {}

bool NullData::operator==(const NullData &other) const { return true; }

Str NullData::toString() const {
    return "null";
}
