#include "KeyMapper.hpp"
#include "serialization.hpp"
#include "objects/Object.hpp"
#include "objects/StringData.hpp"
#include "objects/IndexableData.hpp"
#include "objects/DictData.hpp"

KeyMapper::KeyMapper(const Byte *data, size_t &p) : keyIndices(
        deserialize<Map<Str, Map<Str, Str>>>(data, p)) {}

Bytes KeyMapper::toBytes() const {
    return serialize(keyIndices);
}

bool KeyMapper::addIndex(const Str &index) {
    bool isNew = keyIndices.emplace(index, Map<Str, Str>()).second;
    return isNew;
}

void KeyMapper::removeExtraIndices(const Vec<Str> &validIndices) {
    for (auto it = keyIndices.begin(); it != keyIndices.end();) {
        auto ind = std::find(validIndices.begin(), validIndices.end(), it->first);
        if (ind == validIndices.end()) {
            it = keyIndices.erase(it);
        } else {
            ++it;
        }
    }
}

Str KeyMapper::get(const Str &byKey, const Str &key) {
    auto map = keyIndices[byKey];
    auto objKey = map.find(key);
    if (objKey == map.end()) {
        return "";
    }
    return objKey->second;
}

#include <iostream>


void KeyMapper::registerVal(const Object &val, const Str &key) {
    if (val.has<DictData>()) {
        auto &obj = val.get<DictData>().map;
        for (auto &item : keyIndices) {
            auto &hookedKey = item.first;
            auto &hookedMap = item.second;
            auto objItem = obj.find(hookedKey);
            if (objItem != obj.end() && objItem->second.getId() == STRING_ID) {
                auto hookedVal = objItem->second.get<StringData>().val;
                auto empResult = hookedMap.emplace(hookedVal, key);
                if (!empResult.second && empResult.first->second != key) {  // TODO: Support one-to-many indices
                    throw std::runtime_error("Index is not one to one!");
                }
            }
        }
    }
}

void KeyMapper::unregisterVal(const Object &val, const Str &key) {
    if (val.has<DictData>()) {
        auto &obj = val.get<DictData>().map;
        for (auto &item : keyIndices) {
            auto &hookedKey = item.first;
            auto &hookedMap = item.second;
            auto objItem = obj.find(hookedKey);
            if (objItem != obj.end() && objItem->second.getId() == STRING_ID) {
                auto hookedVal = objItem->second.get<StringData>().val;
                auto it = hookedMap.find(hookedVal);
                if (it == hookedMap.end()) {  // TODO: Figure out why this is needed
                } else {
                    hookedMap.erase(it);
                }
            }
        }
    }
}

bool KeyMapper::hasIndex(const Str &index) {
    return keyIndices.find(index) != keyIndices.end();
}


void KeyMapper::createIndex(const Str &index, const Object &obj) {
    auto &map = keyIndices.emplace(index, Map<Str, Str>()).first->second;
    if (obj.has<IndexableData>()) {
        for (auto &entry : obj.get<IndexableData>().iter()) {
            if (entry.value.has<DictData>()) {
                auto &row = entry.value.get<DictData>().map;
                auto it = row.find(index);
                if (it != row.end() && it->second.has<StringData>()) {
                    auto empResult = map.emplace(it->second.get<StringData>().val, entry.key);
                    if (!empResult.second && empResult.first->second != entry.key) {
                        throw std::runtime_error("Index is not one to one!"); // TODO: Support one-to-many indices
                    }
                }
            }
        }
    }
}
