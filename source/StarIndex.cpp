#include "StarIndex.hpp"
#include "serialization.hpp"
#include "Object.hpp"

StarIndex::StarIndex(const Byte *data, size_t &p) :
        location(deserialize<Vec<Str>>(data, p)),
        pattern(deserialize<Vec<Str>>(data, p)) {}

Bytes StarIndex::toBytes() const {
    return concat(serialize(location), serialize(pattern));
}
