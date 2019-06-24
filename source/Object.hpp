#pragma once

#include <memory>
#include <ostream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cstddef>
#include "serialization.hpp"
#include "Logger.hpp"

#define TO_BYTES(exp, ovr) Bytes toBytes() const ovr { return exp; }
#define TO_BYTES_1(a, ovr) TO_BYTES(serialize(a), ovr)
#define BYTES_CTOR(cls, ...) cls(const Byte *data, size_t &p) : __VA_ARGS__ {}
#define _FB_P(x) x(deserialize<decltype(x)>(data, p))
#define BYTES_CTOR_1(cls, a) BYTES_CTOR(cls, _FB_P(a))

#define VAL_CTOR(cls, a) cls(const decltype(a) &a) : a(a) {}
#define VAL_CTOR_MV(cls, a) cls(decltype(a) a) : a(std::move(a)) {}
#define EQ_OPTOR(cls, a) bool operator==(const cls &other) const { \
        return a == other.a; \
    }

#define OBJ_DATA_FIELD_OVR_NO_VAL_CTOR(cls, a, ovr) \
    TO_BYTES_1(a, ovr) \
    BYTES_CTOR_1(cls, a) \
    cls() = default; \
    EQ_OPTOR(cls, a)

#define OBJ_DATA_FIELD_OVR(cls, a, ovr) \
    OBJ_DATA_FIELD_OVR_NO_VAL_CTOR(cls, a, ovr) \
    VAL_CTOR(cls, a)

#define OBJ_DATA_FIELD_MV(cls, a) \
    OBJ_DATA_FIELD_OVR_NO_VAL_CTOR(cls, a, override) \
    VAL_CTOR_MV(cls, a)


#define OBJ_DATA_FIELD(cls, a) OBJ_DATA_FIELD_OVR(cls, a, override)

struct ObjData {
    virtual Bytes toBytes() const = 0;
    virtual Str toString() const = 0;
    virtual ~ObjData() = default;
};

enum ObjType : size_t {
    EMPTY_ID = 0,
    NULL_ID = 1,
    BOOL_ID = 2,
    INT_ID = 3,
    FLOAT_ID = 4,
    STRING_ID = 5,
    LIST_ID = 6,
    DICT_ID = 7,
};

class DictData;

class ListData;
class IntData;

class Object {
public:
    using Ptr = std::unique_ptr<Object>;

    Object() : id(EMPTY_ID), data(nullptr) {}

    Object(size_t id);
    Object(const Str &str);
    Object(const char *str);
    Object(float val);
    Object(bool val);
    Object(int val);
    Object(std::nullptr_t val);
    Object(const DictData &data);
    Object(const Map<Str, Object> &data);
    Object(const ListData &data);
    Object(const Vec<Object> &data);
    Object(const Byte *data, size_t &p);
    Object(const Object &other);
    Object(Object &&other) noexcept;
    Object &operator=(Object other);
    bool operator==(const Object &other) const;
    bool operator!=(const Object &other) const;
    ~Object();

    explicit operator bool() const {
        return bool(data);
    }

    bool isEmpty() const {
        return !data;
    }

    friend void swap(Object &a, Object &b);
    size_t getId() const;
    bool is(size_t typeId) const;
    Bytes toBytes() const;
    Str toString() const;
    static Object parse(const Str &s);

    template<typename R>
    R &get();
    template<typename R>
    const R &get() const;
    template<typename R>
    const bool has() const;

    friend std::ostream &operator<<(std::ostream &stream, const Object &object) {
        stream << object.toString();
        return stream;
    }

private:
    static Object parseFromString(const char *&c);
    static void skipSpaces(const char *&c);

    ObjData *defaultData(size_t id);
    ObjData *deserializeData(const Byte *data, size_t &p);
    ObjData *cloneData() const;

    size_t id;
    ObjData *data{};
};

struct StringData : public ObjData {
    Str val;

    static Str escaped(const Str &s) {
        std::stringstream ss;
        const char *cstr = s.c_str();
        for (const char *c = cstr; c != cstr + s.size(); ++c) {
            switch (*c) {
            case '\\':
                ss << "\\\\";
                break;
            case '\n':
                ss << "\\n";
                break;
            case '"':
                ss << "\\\"";
                break;
            default:
                ss << *c;
                break;
            }
        }
        return ss.str();
    }

    Str toString() const override {
        return '"' + escaped(val) + '"';
    }

    OBJ_DATA_FIELD_MV(StringData, val);
};

struct FloatData : public ObjData {
    float val{};

    Str toString() const override {
        char buf[64];
        sprintf(buf, "%-.*g", 8, val);
        return buf;
    }

    OBJ_DATA_FIELD(FloatData, val);
};

struct IntData : public ObjData {
    int val{};

    Str toString() const override {
        return std::to_string(val);
    }

    OBJ_DATA_FIELD(IntData, val);

};

struct BoolData : public ObjData {
    bool val{};

    Str toString() const override {
        return val ? "true" : "false";
    }

    OBJ_DATA_FIELD(BoolData, val);
};

struct NullData : public ObjData {
    NullData() = default;

    NullData(const Byte *data, size_t &p) {}

    bool operator==(const NullData &other) const { return true; }

    Str toString() const override {
        return "null";
    }

    TO_BYTES(Bytes(), override);
};

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

struct ListData : public ObjData, public IndexableData {
    Vec<Object> list;

    Str toString() const override {
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

    std::pair<bool, int> parseIndex(const Str &key) const {
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

    bool hasKey(const Str &key) const override {
        return parseIndex(key).first;
    }

    Object &operator[](const Str &key) override {
        return list[parseIndex(key).second];
    }

    const Object &operator[](const Str &key) const override {
        return list[parseIndex(key).second];
    }

    Vec<Entry> iter() override {
        Vec<Entry> items;
        items.reserve(list.size());
        for (auto i = 0; i < list.size(); ++i) {
            items.emplace_back(Entry{std::to_string(i), list[i]});
        }
        return items;
    }

    Vec<ConstEntry> iter() const override {
        Vec<ConstEntry> items;
        items.reserve(list.size());
        for (auto i = 0; i < list.size(); ++i) {
            items.emplace_back(ConstEntry{std::to_string(i), list[i]});
        }
        return items;
    }

    Object *add(const Str &key, const Object &obj) override {
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

    OBJ_DATA_FIELD_MV(ListData, list);
};

using DictMap = Map<Str, Object>;

struct DictData : public ObjData, public IndexableData {
    DictMap map;

    Str toString() const override {
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

    bool hasKey(const Str &key) const override {
        return map.find(key) != map.end();
    }

    Object &operator[](const Str &key) override {
        return map.at(key);
    }

    const Object &operator[](const Str &key) const override {
        return map.at(key);
    }

    Vec<Entry> iter() override {
        Vec<Entry> items;
        items.reserve(map.size());
        for (auto &i : map) {
            items.emplace_back(Entry{i.first, i.second});
        }
        return items;
    }

    Vec<ConstEntry> iter() const override {
        Vec<ConstEntry> items;
        items.reserve(map.size());
        for (auto &i : map) {
            items.emplace_back(ConstEntry{i.first, i.second});
        }
        return items;
    }

    Object *add(const Str &key, const Object &obj) override {
        auto res = map.emplace(key, obj);
        if (!res.second) {
            lg.info("Creating object where one already exists");
            return nullptr;
        }
        return &res.first->second;
    }

    OBJ_DATA_FIELD_MV(DictData, map);
};

// Template implementation

template<typename R>
R &Object::get() {
    auto res = dynamic_cast<R *>(data);
    if (!res) {
        throw std::runtime_error("Getting data of wrong type");
    }
    return *res;
}

template<typename R>
const R &Object::get() const {
    auto res = dynamic_cast<R *>(data);
    if (!res) {
        throw std::runtime_error("Getting data of wrong type");
    }
    return *res;
}


template<typename R>
const bool Object::has() const {
    return bool(dynamic_cast<R *>(data));
}
