#pragma once

#include "utils.hpp"
#include "IndexNode.hpp"

struct StarIndex {
    Vec<Str> location;
    Vec<Str> pattern;
    IndexNode *node{};

    StarIndex(Vec<Str> location, Vec<Str> pattern, IndexNode &node) :
            location(std::move(location)), pattern(std::move(pattern)), node(&node) {}

    StarIndex(const Byte *data, size_t &p);

    Bytes toBytes() const;

    bool operator==(const StarIndex &other) {
        return location == other.location && pattern == other.pattern;
    }

    bool operator!=(const StarIndex &other) {
        return !(*this == other);
    }
};

