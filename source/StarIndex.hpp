#pragma once

#include "utils.hpp"
#include "IndexNode.hpp"

struct StarIndex {
    Vec<Str> location;
    Vec<Str> pattern;
    IndexNode *node{};

    StarIndex(Vec<Str> location, Vec<Str> pattern, IndexNode &node);

    StarIndex(const Byte *data, size_t &p);

    Bytes toBytes() const;

    bool operator==(const StarIndex &other);

    bool operator!=(const StarIndex &other);
};

