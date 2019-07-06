#include "ListData.hpp"
#include "Logger.hpp"


Str ListData::toString() const {
    Vec<Str> parts = {"["};
    if (!list.empty()) {
        for (auto &i : list) {
            parts.emplace_back(i.toString());
            parts.emplace_back(", ");
        }
        parts.pop_back();
    }
    parts.push_back("]");
    return concat(parts);
}

std::pair<bool, int> ListData::parseIndex(const Str &key) const {
    char *p;
    errno = 0;
    long converted = strtol(key.c_str(), &p, 10);
    bool isNum = !*p;
    if (key.empty() || errno != 0 || *p) {
        return {false, 0};
    }
    auto num = converted;
    if (num < 0) {
        num += list.size();
    }
    if (num < 0 || num >= list.size()) {
        return {false, 0};
    }
    return {true, num};
}

bool ListData::hasKey(const Str &key) const {
    return parseIndex(key).first;
}

Object &ListData::operator[](const Str &key) {
    return list[parseIndex(key).second];
}

const Object &ListData::operator[](const Str &key) const {
    return list[parseIndex(key).second];
}

Vec<IndexableData::Entry> ListData::iter() {
    Vec<Entry> items;
    items.reserve(list.size());
    for (auto i = 0; i < list.size(); ++i) {
        items.emplace_back(Entry{std::to_string(i), list[i]});
    }
    return items;
}

Vec<IndexableData::ConstEntry> ListData::iter() const {
    Vec<ConstEntry> items;
    items.reserve(list.size());
    for (auto i = 0; i < list.size(); ++i) {
        items.emplace_back(ConstEntry{std::to_string(i), list[i]});
    }
    return items;
}

Object *ListData::add(const Str &key, const Object &obj) {
    auto res = parseIndex(key);
    auto id = res.first ? res.second : -1;
    if (id < 0) {
        lg.info("Attempting to create at invalid list key");
        return nullptr;
    }
    if (id != list.size()) {
        lg.info("Attempting to create at list key inside list");
        return nullptr;
    }
    list.emplace_back(obj);
    return &list.back();
}
