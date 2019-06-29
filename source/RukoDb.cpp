#include <iostream>
#include <algorithm>
#include <fstream>
#include "RukoDb.hpp"
#include "IndexManager.hpp"
#include "Logger.hpp"
#include "objects/IndexableData.hpp"
#include "objects/ListData.hpp"
#include "objects/DictData.hpp"
#include "objects/StringData.hpp"


static Object filterFields(const Object &dict, const Vec<Str> &fields, const Vec<Str> &exclude) {
    auto &src = dict.get<DictData>().map;
    Object result(DICT_ID);
    auto &data = result.get<DictData>().map;
    data.reserve(src.size());
    for (auto &item : src) {
        if ((exclude.empty() && std::find(fields.begin(), fields.end(), item.first) != fields.end()) ||
            (fields.empty() && std::find(exclude.begin(), exclude.end(), item.first) == exclude.end())) {
            data.emplace(item.first, item.second);
        }
    }
    return result;
}

static Object cloneIndexableBy(const Object &obj, const Str &byIndex, const Vec<Str> &fields = {},
                               const Vec<Str> &exclude = {}) {
    Object dict(DICT_ID);
    auto &dictMap = dict.get<DictData>().map;
    if (obj.has<IndexableData>()) {
        for (auto &item : obj.get<IndexableData>().iter()) {
            if (item.value.has<DictData>()) {
                auto &map = item.value.get<DictData>().map;
                auto val = map.find(byIndex);
                if (val != map.end() && val->second.has<StringData>()) {
                    auto element = item.value;
                    if (!fields.empty() || !exclude.empty()) {
                        element = filterFields(element, fields, exclude);
                    }
                    dictMap.emplace(val->second.get<StringData>().val, element);
                }
            }
        }
    } else {
        throw std::runtime_error("Type not indexable!");
    }
    return dict;
}

RukoDb::RukoDb(const Str &filename, bool saveJson) : filename(filename), saveJson(saveJson) {
    if (!filename.empty()) {
        try {
            load();
            if (saveJson) {
                save();
            }
        } catch (const std::exception &e) {
            this->filename.clear();
            std::cerr << "Warning: failed to load database. Saving disabled." << std::endl;
        }
    }
}

Object RukoDb::get(Vec<Str> keys, const Vec<Str> &fields, const Vec<Str> &exclude) {
    std::lock_guard<std::mutex> lock(mut);
    Str lastBy;
    if (!keys.empty()) {
        auto key = keys[keys.size() - 1];
        if (!key.empty() && key[key.size() - 1] == ':') {
            lastBy = key.substr(0, key.size() - 1);
            keys.pop_back();
        }
    }
    auto frames = indices.traverseTo(keys, db, false, false);
    Object *obj = frames.back().obj;
    if (!lastBy.empty()) {
        bool noIndex = !frames.back().index || !*frames.back().index;
        if (noIndex && (!obj || !obj->has<IndexableData>())) {
            lg.info("No indexable data for last index");
            return {};
        }
        auto res = indices.getMappingRepresentation(frames, lastBy, db);
        if (res.isEmpty()) {
            return cloneIndexableBy(*obj, lastBy, fields, exclude);
        }
        return res;
    }
    if (!obj) {
        return {};
    }

    if ((!fields.empty() || !exclude.empty()) && obj->is(DICT_ID)) {
        return filterFields(*obj, fields, exclude);
    } else {
        return *obj;
    }
}

bool RukoDb::set(const Vec<Str> &keys, const Object &value) {
    std::lock_guard<std::mutex> lock(mut);
    auto frames = indices.traverseTo(keys, db, false, true);
    if (frames.back().invalid(false, true)) {
        return false;
    }
    indices.handleDelete(frames, db);
    *frames.back().obj = value;
    indices.handleCreate(frames, db);
    return true;
}

bool RukoDb::lput(const Vec<Str> &keys, const Object &value) {
    {
        std::lock_guard<std::mutex> lock(mut);
        auto frames = indices.traverseTo(keys, db, false, true);
        if (frames.back().invalid(false, true)) {
            return false;
        }
        auto &obj = *frames.back().obj;
        if (obj.isEmpty()) {
            obj = Object(LIST_ID);
        }
        if (!obj.has<ListData>()) {
            return false;
        }

        auto &list = obj.get<ListData>().list;
        list.emplace_back(value);
        frames.emplace_back(frames.back().traverse(std::to_string(list.size() - 1)));
        indices.handleCreate(frames, db);
    }
    return true;
}

bool RukoDb::del(const Vec<Str> &keys) {
    std::lock_guard<std::mutex> lock(mut);

    auto frames = indices.traverseTo(keys, db);
    if (frames.back().invalid(false)) {
        return false;
    }
    if (frames.size() == 1) {  // Deleting root node
        indices.handleDelete(frames, db);
        db = Object(DICT_ID);
        return true;
    }
    eassert(frames.size() >= 2);
    auto &parentFrame = frames[frames.size() - 2];
    auto &parent = *parentFrame.obj;
    eassert(parent.has<IndexableData>());

    if (parent.has<DictData>()) {
        auto &map = parent.get<DictData>().map;
        auto it = map.find(frames.back().key);
        if (it != map.end()) {
            indices.handleDelete(frames, db);
            map.erase(it);
            return true;
        }
    } else {
        auto &list = parent.get<ListData>().list;
        int id = parseIndex(frames.back().key, list.size());
        if (id >= 0) {
            frames.pop_back();
            for (auto i = id; i < list.size(); ++i) {  // Keys shift for numbers in front of id
                frames.emplace_back(frames.back().traverse(std::to_string(i), false));
                indices.handleDelete(frames, db);
                frames.pop_back();
            }
            list.erase(list.begin() + id);
            for (auto i = id; i < list.size(); ++i) {
                frames.emplace_back(frames.back().traverse(std::to_string(i), false));
                indices.handleCreate(frames, db);
                frames.pop_back();
            }
            return true;
        }
    }

    return false;
}

void RukoDb::save(Str filename) {
    filename = withDefaultFilename(filename);
    if (filename.empty()) {
        return;
    }
    Bytes data;
    {
        std::lock_guard<std::mutex> lock(mut);
        data = concat(db.toBytes(), indices.toBytes());
    }
    {
        std::ofstream of(filename, std::ios::out | std::ios::binary);
        of.write(bytesToPointer<char>(data), data.size());
    }
    if (saveJson) {
        std::ofstream of(filename + ".json", std::ios::out);
        auto s = db.toString();
        of.write(s.c_str(), s.size());
    }
}

bool RukoDb::load(Str filename) {
    filename = withDefaultFilename(filename);
    std::ifstream dbFile(filename, std::ios::binary | std::ios::ate);
    if (!dbFile.is_open()) {
        return false;
    }
    auto size = dbFile.tellg();
    dbFile.seekg(0, std::ios::beg);
    Bytes bytes((size_t) size);
    dbFile.read(reinterpret_cast<char *>(bytes.data()), size);

    std::lock_guard<std::mutex> lock(mut);
    const Byte *data = bytes.data();
    size_t p = 0;
    db = Object(data, p);
    indices = IndexManager(data, p);
    return true;
}

bool RukoDb::declare(const Vec<Str> &keys, size_t dataType, const Vec<Str> &indices) {

    std::lock_guard<std::mutex> lock(mut);
    auto frames = this->indices.traverseTo(keys, db, false, true);
    if (frames.back().invalid(false, true)) {
        return false;
    }
    auto &obj = *frames.back().obj;
    if (obj.getId() == dataType) {
        return false;
    }
    if (!obj.isEmpty() && obj.getId() != dataType) {
        lg.warning("declaring key over key of wrong type. Removing old value: " + obj.toString());
        this->indices.handleDelete(frames, db);
    }
    obj = Object(dataType);
    this->indices.handleCreate(frames, db);
    return true;
}

bool RukoDb::createMapping(const Vec<Str> &location, const Vec<Str> &filter) {
    return indices.createMapping(location, filter, db);
}

bool RukoDb::deleteMapping(const Vec<Str> &location) {
    return indices.deleteMapping(location, db);
}

Object RukoDb::getMappings() {
    return indices.getMappings();
}

Str RukoDb::withDefaultFilename(Str filename) const {
    if (filename.empty()) {
        filename = this->filename;
    }
    return filename;
}

int RukoDb::parseIndex(const Str &indexStr, size_t n) {
    int index = std::stoi(indexStr);
    if (index < 0) {
        index += n;
    }
    if (index < 0 || index >= n) {
        return -1;
    }
    return index;
}
