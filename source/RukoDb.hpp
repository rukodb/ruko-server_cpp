#include <utility>

#pragma once

#include <mutex>
#include "Object.hpp"
#include "IndexManager.hpp"

class RukoDb {
public:
    RukoDb() = default;
    RukoDb(const Str &filename, bool saveJson = false);

    Object get(Vec<Str> keys, const Vec<Str> &fields = {}, const Vec<Str> &exclude = {});
    bool set(const Vec<Str> &keys, const Object &value);
    bool lput(const Vec<Str> &keys, const Object &value);
    bool del(const Vec<Str> &keys);
    bool declare(const Vec<Str> &keys, size_t dataType, const Vec<Str> &indices = {});
    void save(Str filename = "");
    bool load(Str filename = "");
    bool createMapping(const Vec<Str> &location, const Vec<Str> &filter);
    bool deleteMapping(const Vec<Str> &location);
    Object getMappings();

private:
    Str withDefaultFilename(Str filename) const;

    static int parseIndex(const Str &key, size_t n);

    Object db{DICT_ID};
    std::mutex mut;
    Str filename;
    bool saveJson = false;
    IndexManager indices;
};
