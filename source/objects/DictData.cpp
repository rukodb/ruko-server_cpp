#include "DictData.hpp"
#include "Logger.hpp"


Str DictData::toString() const {
    Vec<Str> parts = {"{"};
    if (!map.empty()) {
        for (auto &i : map) {
            parts.emplace_back("\"" + i.first + "\"");
            parts.emplace_back(": ");
            parts.emplace_back(i.second.toString());
            parts.emplace_back(", ");
        }
        parts.pop_back();
    }
    parts.push_back("}");
    return concat(parts);
}

bool DictData::hasKey(const Str &key) const {
    return map.find(key) != map.end();
}

Object &DictData::operator[](const Str &key) {
    return map.at(key);
}

const Object &DictData::operator[](const Str &key) const {
    return map.at(key);
}

Vec<IndexableData::Entry> DictData::iter() {
    Vec<Entry> items;
    items.reserve(map.size());
    for (auto &i : map) {
        items.emplace_back(Entry{i.first, i.second});
    }
    return items;
}

Vec<IndexableData::ConstEntry> DictData::iter() const {
    Vec<ConstEntry> items;
    items.reserve(map.size());
    for (auto &i : map) {
        items.emplace_back(ConstEntry{i.first, i.second});
    }
    return items;
}

Object *DictData::add(const Str &key, const Object &obj) {
    auto res = map.emplace(key, obj);
    if (!res.second) {
        lg.info("Creating object where one already exists");
        return nullptr;
    }
    return &res.first->second;
}
