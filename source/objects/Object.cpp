#include <cmath>
#include "Object.hpp"
#include "IntData.hpp"
#include "BoolData.hpp"
#include "NullData.hpp"
#include "DictData.hpp"
#include "serialization.hpp"
#include "StringData.hpp"
#include "FloatData.hpp"
#include "ListData.hpp"

#define WITH_OBJ_TYPE(id, expr) switch (id) { \
    case NULL_ID: { \
        using Type = NullData; \
        expr; \
    } \
    case BOOL_ID: { \
        using Type = BoolData; \
        expr; \
    } \
    case INT_ID: { \
        using Type = IntData; \
        expr; \
    } \
    case FLOAT_ID: { \
        using Type = FloatData; \
        expr; \
    } \
    case STRING_ID: { \
        using Type = StringData; \
        expr; \
    } \
    case LIST_ID: { \
        using Type = ListData; \
        expr; \
    } \
    case DICT_ID: { \
        using Type = DictData; \
        expr; \
    } \
    default: \
        throw std::invalid_argument("Invalid object id: " + std::to_string(id)); \
}

Object::Object(size_t id) : id(id), data(defaultData(id)) {}

Object::Object(const Str &str) : id(STRING_ID), data(new StringData(str)) {}

Object::Object(const char *str) : id(STRING_ID), data(new StringData(str)) {}

Object::Object(float val) : id(FLOAT_ID), data(new FloatData(val)) {}

Object::Object(bool val) : id(BOOL_ID), data(new BoolData(val)) {}

Object::Object(int val) : id(INT_ID), data(new IntData(val)) {}

Object::Object(std::nullptr_t val) : id(NULL_ID), data(new NullData()) {}

Object::Object(const DictData &data) : id(DICT_ID), data(new DictData(data)) {}
Object::Object(const DictMap &data) : id(DICT_ID), data(new DictData(data)) {}
Object::Object(const ListData &data) : id(LIST_ID), data(new ListData(data)) {}
Object::Object(const Vec<Object> &data) : id(LIST_ID), data(new ListData(data)) {}

Object::Object(const Byte *data, size_t &p) :
        id(deserialize<Byte>(data, p)), data(deserializeData(data, p)) {}

Object::Object(const Object &other) : id(other.id), data(other.cloneData()) {}

Object::Object(Object &&other) noexcept : id(NULL_ID), data(nullptr) {
    swap(*this, other);
}

Object &Object::operator=(Object other) {
    swap(*this, other);
    return *this;
}

bool Object::operator==(const Object &other) const {
    if (id != other.id) {
        return false;
    } else if (id == EMPTY_ID) {
        return true;
    } else {
        WITH_OBJ_TYPE(id, return get<Type>() == other.get<Type>(););
    }
}

bool Object::operator!=(const Object &other) const {
    return !(*this == other);
}

void swap(Object &a, Object &b) {
    using std::swap;
    swap(a.id, b.id);
    swap(a.data, b.data);
}

Object::~Object() {
    delete data;
}

Object::operator bool() const {
    return bool(data);
}

bool Object::isEmpty() const {
    return !data;
}

std::ostream &operator<<(std::ostream &stream, const Object &object) {
    stream << object.toString();
    return stream;
}

size_t Object::getId() const {
    return id;
}

bool Object::is(size_t typeId) const {
    return id == typeId;
}

Bytes Object::toBytes() const {
    if (id == 0) {
        return serialize((Byte) id);
    }
    return serialize((Byte) id) + data->toBytes();
}

Str Object::toString() const {
    return data ? data->toString() : Str("?");
}

ObjData *Object::defaultData(size_t id) {
    if (id == 0) {
        return nullptr;
    }
    WITH_OBJ_TYPE(id, return new Type());
}

ObjData *Object::deserializeData(const Byte *data, size_t &p) {
    if (id == 0) {
        return nullptr;
    }
    WITH_OBJ_TYPE(id, return new Type(data, p));
}

ObjData *Object::cloneData() const {
    if (!data) {
        return nullptr;
    }
    WITH_OBJ_TYPE(id, return new Type(get<Type>()));
}

Object Object::parse(const Str &s) {
    const char *c = s.c_str();
    skipSpaces(c);
    return parseFromString(c);
}

Object Object::parseFromString(const char *&c) {
    switch (*c++) {
        case '{': {
            skipSpaces(c);
            Object object(DICT_ID);
            auto &map = object.get<DictData>().map;
            while (*c != '}') {
                auto key = parseFromString(c);
                if (key.getId() != STRING_ID) {
                    throw std::runtime_error("Key in dictionary is not a string!");
                }
                if (*c == ':') {
                    ++c;
                    skipSpaces(c);
                } else {
                    throw std::runtime_error("No colon after value in dictionary!");
                }
                auto val = parseFromString(c);
                char a = *c;
                if (a != ',' && a != '}') {
                    throw std::runtime_error("No comma or brace after value in dictionary!");
                }
                if (a == ',') {
                    ++c;
                    skipSpaces(c);
                }
                map.emplace(key.get<StringData>().val, val);
            }
            ++c;
            skipSpaces(c);
            return object;
        }
        case '[': {
            skipSpaces(c);
            Object object(LIST_ID);
            auto &list = object.get<ListData>().list;
            while (*c != ']') {
                auto val = parseFromString(c);
                char a = *c;
                if (a != ',' && a != ']') {
                    throw std::runtime_error("No comma or square bracket after value in array!");
                }
                if (a == ',') {
                    ++c;
                    skipSpaces(c);
                }
                list.emplace_back(val);
            }
            ++c;
            skipSpaces(c);
            return object;
        }
        case '"': {
            const char *start = c;
            while (*c != '"') {
                if (*c == '\0') {
                    throw std::runtime_error("Json cuts off with open string");
                }
                ++c;
            }
            const char *end = c;
            ++c;
            skipSpaces(c);
            return Object(Str(start, end));
        }
        case 't': {
            if (*c++ != 'r' || *c++ != 'u' || *c++ != 'e') {
                throw std::runtime_error("Invalid expression starting with t!");
            }
            skipSpaces(c);
            return Object(true);
        }
        case 'f': {
            if (*c++ != 'a' || *c++ != 'l' || *c++ != 's' || *c++ != 'e') {
                throw std::runtime_error("Invalid expression starting with f!");
            }
            skipSpaces(c);
            return Object(false);
        }
        case 'n': {
            if (*c++ != 'u' || *c++ != 'l' || *c++ != 'l') {
                throw std::runtime_error("Invalid expression starting with n!");
            }
            skipSpaces(c);
            return Object(nullptr);
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.': {
            --c;
            const char *first = c;
            int number = parseRawInt(c);
            if (*c != '.') {
                number *= parseSciMult(c);
                skipSpaces(c);
                return Object(number);
            }
            ++c;
            double f = number;
            int digit = 10;
            while (true) {
                int d = *c - '0';
                if (d > 9 || d < 0) {
                    break;
                }
                ++c;
                f += d / double(digit);
                digit *= 10;
            }
            if (c == first) {
                throw std::runtime_error("Decimal without any digits!");
            }
            f *= parseSciMult(c);
            skipSpaces(c);
            return Object(float(f));
        }
        default: {
            throw std::runtime_error("Invalid start character of value: " + std::string(1, *--c));
        }
    }
}

void Object::skipSpaces(const char *&c) {
    while (std::isspace(*c)) {
        ++c;
    }
}

int Object::parseRawInt(const char *&c) {
    int number = 0;
    while (*c != '.') {
        int d = *c - '0';
        if (d > 9 || d < 0) {
            break;
        }
        ++c;
        number = number * 10 + d;
    }
    return number;
}

int Object::parseSciMult(const char *&c) {
    if (*c == 'e') {  // Scientific notation
        ++c;
        int sign = 1;
        if (*c == '+') {
            ++c;
        } else if (*c == '-') {
            sign = -1;
            ++c;
        }
        auto begin = c;
        auto power = sign * parseRawInt(c);
        if (c == begin) {
            throw std::runtime_error("No integer after scientific notation!");
        }
        return std::pow(10, power);
    }
    return 1;
}
