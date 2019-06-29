#pragma once

#include <utils.hpp>
#include "Object.hpp"

class IndexableData {
public:
    struct Entry {
        Str key;
        Object &value;
    };
    struct ConstEntry {
        Str key;
        const Object &value;
    };
    virtual Object &operator[](const Str &key) = 0;
    virtual const Object &operator[](const Str &key) const = 0;
    virtual bool hasKey(const Str &key) const = 0;
    virtual Vec<Entry> iter() = 0;
    virtual Vec<ConstEntry> iter() const = 0;
    virtual Object *add(const Str &key, const Object &obj) = 0;
};
