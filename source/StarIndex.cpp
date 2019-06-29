#include "StarIndex.hpp"
#include "serialization.hpp"
#include "objects/Object.hpp"


StarIndex::StarIndex(Vec<Str> location, Vec<Str> pattern, IndexNode &node) :
        location(std::move(location)), pattern(std::move(pattern)), node(&node) {}

StarIndex::StarIndex(const Byte *data, size_t &p) :
        location(deserialize<Vec<Str>>(data, p)),
        pattern(deserialize<Vec<Str>>(data, p)) {}

Bytes StarIndex::toBytes() const {
    return concat(serialize(location), serialize(pattern));
}

bool StarIndex::operator==(const StarIndex &other) {
    return location == other.location && pattern == other.pattern;
}

bool StarIndex::operator!=(const StarIndex &other) {
    return !(*this == other);
}
