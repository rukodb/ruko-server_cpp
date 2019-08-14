#pragma once

#include <cstddef>
#include <memory>
#include "serialization.hpp"
#include "utils.hpp"

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


class ObjData;

class DictData;

class ListData;


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

    explicit operator bool() const;

    bool isEmpty() const;

    friend void swap(Object &a, Object &b);
    size_t getId() const;
    bool is(size_t typeId) const;
    Bytes toBytes() const;
    Str toString() const;
    static Object parse(const Str &s);
    static Object parseFromString(const char *&c);

    template<typename R>
    R &get();
    template<typename R>
    const R &get() const;
    template<typename R>
    const bool has() const;

    friend std::ostream &operator<<(std::ostream &stream, const Object &object);

private:
    static void skipSpaces(const char *&c);
    static int parseRawInt(const char *&c);
    static int parseSciMult(const char *&c);

    ObjData *defaultData(size_t id);
    ObjData *deserializeData(const Byte *data, size_t &p);
    ObjData *cloneData() const;

    size_t id;
    ObjData *data{};
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
