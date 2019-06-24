#pragma once
#include <memory>
#include <algorithm>
#include "utils.hpp"

class Object;

class KeyMapper {
public:
    using Ptr = std::unique_ptr<KeyMapper>;

    KeyMapper() = default;
    KeyMapper(const Byte *data, size_t &p);

    bool addIndex(const Str &index);
    bool hasIndex(const Str &index);
    void createIndex(const Str &index, const Object &obj);
    void removeExtraIndices(const Vec<Str> &validIndices);
    Str get(const Str &byKey, const Str &key);
    Bytes toBytes() const;

    void registerVal(const Object &val, const Str &key);
    void unregisterVal(const Object &val, const Str &key);

// TODO: Make private
    Map<Str, Map<Str, Str>> keyIndices;
};
