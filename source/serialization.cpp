#include "serialization.hpp"
#include "objects/Object.hpp"
#include "StarIndex.hpp"


Bytes withLen(const Bytes &bytes) {
    return concat(u32ToBytes(uint32_t(bytes.size())), bytes);
}


template<typename T>
Bytes serializePrimitive(T t) {
    auto *p = reinterpret_cast<Byte const *>(&t);
    return Bytes(p, p + sizeof(T));
}

template<typename T>
T deserializePrimitive(const Byte *data, size_t &p) {
    T t;
    auto *tData = reinterpret_cast<Byte*>(&t);
    for (auto i = 0; i < sizeof(t) / sizeof(Byte); ++i) {
        tData[i] = data[i + p];
    }
    p += sizeof(t) / sizeof(Byte);
    return t;
}

Bytes serialize(float f) {
    return serializePrimitive(f);
}

Bytes serialize(Byte b) {
    return {b};
}

Bytes serialize(size_t n) {
    return u32ToBytes(uint32_t(n));
}

Bytes serialize(int n) {
    return i32ToBytes(n);
}

Bytes serialize(bool n) {
    return {(Byte) n};
}

Bytes serialize(void *) {
    return {};
}

template<>
float deserialize<float>(const Byte *data, size_t &p) {
    return deserializePrimitive<float>(data, p);
}

template<>
Byte deserialize<Byte>(const Byte *data, size_t &p) {
    p += 1;
    return data[p - 1];
}

template<>
size_t deserialize<size_t>(const Byte *data, size_t &p) {
    return bytesToU32(data, p);
}

template<>
int deserialize<int>(const Byte *data, size_t &p) {
    return bytesToI32(data, p);
}

template<>
bool deserialize<bool>(const Byte *data, size_t &p) {
    return deserialize<Byte>(data, p) != 0;
}

template<>
void *deserialize<void *>(const Byte *data, size_t &p) {
    return nullptr;
}

template<typename K, typename V>
Bytes serializeMap(const std::unordered_map<K, V> &map) {
    Bytes bs = u32ToBytes(uint32_t(map.size()));
    for (auto &i : map) {
        bs += serialize(i.first);
        bs += serialize(i.second);
    }
    return bs;
}

template<typename K, typename V>
std::unordered_map<K, V> deserializeMap(const Byte *data, size_t &p) {
//    EASY_FUNCTION();

    auto len = deserialize<size_t>(data, p);
    Map<K, V> map{};
    map.reserve(len);
    for (int i = 0; i < len; ++i) {
        auto key = deserialize<K>(data, p);
        auto value = deserialize<V>(data, p);
        map[key] = std::move(value);
    }
    return map;
}

Bytes serialize(const Vec<Str> &vec) {
    auto bytes = serialize(vec.size());
    for (auto &i : vec) {
        bytes += serialize(i);
    }
    return bytes;
}

Bytes serialize(const Vec<size_t> &vec) {
    auto bytes = serialize(vec.size());
    for (auto &i : vec) {
        bytes += serialize(i);
    }
    return bytes;
}

Bytes serialize(const Set<Str> &vec) {
    auto bytes = serialize(vec.size());
    for (auto &i : vec) {
        bytes += serialize(i);
    }
    return bytes;
}

Bytes serialize(const Set<size_t> &vec) {
    auto bytes = serialize(vec.size());
    for (auto &i : vec) {
        bytes += serialize(i);
    }
    return bytes;
}

Bytes serialize(const Map<Str, Object> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, IndexNode::Ptr> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Set<Str>> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Str> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Set<size_t>> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Map<Str, Set<Str>>> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Map<Str, Str>> &map) {
    return serializeMap(map);
}

Bytes serialize(const Map<Str, Map<Str, Set<size_t>>> &map) {
    return serializeMap(map);
}

template<>
Map<Str, Map<Str, Set<size_t>>>
deserialize<Map<Str, Map<Str, Set<size_t>>>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Map<Str, Set<size_t>>>(data, p);
}

template<>
Map<Str, Map<Str, Set<Str>>>
deserialize<Map<Str, Map<Str, Set<Str>>>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Map<Str, Set<Str>>>(data, p);
}

template<>
Map<Str, Map<Str, Str>>
        deserialize<Map<Str, Map<Str, Str>>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Map<Str, Str>>(data, p);
}

template<>
Map<Str, Set<size_t>> deserialize<Map<Str, Set<size_t>>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Set<size_t>>(data, p);
}

template<>
Map<Str, Set<Str>> deserialize<Map<Str, Set<Str>>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Set<Str>>(data, p);
}

template<>
Map<Str, IndexNode::Ptr> deserialize<Map<Str, IndexNode::Ptr>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, IndexNode::Ptr>(data, p);
}

template<>
Map<Str, Str> deserialize<Map<Str, Str>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Str>(data, p);
}

template<>
Map<Str, Object> deserialize<Map<Str, Object>>(const Byte *data, size_t &p) {
    return deserializeMap<Str, Object>(data, p);
}

Bytes serialize(const Object &obj) {
    if (obj.isEmpty()) {
        throw std::runtime_error("Serializing empty object");
    }
    return obj.toBytes();
}

Bytes serialize(const StarIndex &index) {
    return index.toBytes();
}

template<>
Object deserialize<Object>(const Byte *data, size_t &p) {
    return Object(data, p);
}

template<>
StarIndex deserialize<StarIndex>(const Byte *data, size_t &p) {
    return StarIndex(data, p);
}

Bytes serialize(const char *str) {
    auto *data = reinterpret_cast<const Byte *>(str);
    return withLen(Bytes(data, data + strlen(str)));
}

Bytes serialize(const Str &str) {
    return serialize(str.c_str());
}

Bytes serialize(const IndexNode::Ptr &node) {
    return node->toBytes();
}

template<>
IndexNode::Ptr deserialize<IndexNode::Ptr>(const Byte *data, size_t &p) {
    return std::make_unique<IndexNode>(data, p);
}

template<>
Str deserialize<Str>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    auto *chars = reinterpret_cast<const char*>(data);
    auto s = Str(chars + p, chars + p + len);
    p += len;
    return s;
}

Bytes serialize(const Vec<Object> &data) {
    Bytes bs = u32ToBytes(uint32_t(data.size()));
    for (auto &i : data) {
        bs += serialize(i);
    }
    return bs;
}

Bytes serialize(const Vec<StarIndex> &data) {
    Bytes bs = u32ToBytes(uint32_t(data.size()));
    for (auto &i : data) {
        bs += serialize(i);
    }
    return bs;
}

template<>
Vec<Object> deserialize<Vec<Object>>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    Vec<Object> objects;
    objects.reserve(len);
    for (int i = 0; i < len; ++i) {
        objects.emplace_back(std::move(deserialize<Object>(data, p)));
    }
    return objects;
}

template<>
Vec<StarIndex> deserialize<Vec<StarIndex>>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    Vec<StarIndex> objects;
    objects.reserve(len);
    for (int i = 0; i < len; ++i) {
        objects.emplace_back(std::move(deserialize<StarIndex>(data, p)));
    }
    return objects;
}

template<>
Vec<Str> deserialize<Vec<Str>>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    Vec<Str> objects;
    objects.reserve(len);
    for (int i = 0; i < len; ++i) {
        objects.emplace_back(std::move(deserialize<Str>(data, p)));
    }
    return objects;
}

template<>
Set<Str> deserialize<Set<Str>>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    Set<Str> objects{};
    objects.reserve(len);
    for (int i = 0; i < len; ++i) {
        objects.emplace(std::move(deserialize<Str>(data, p)));
    }
    return objects;
}

template<>
Set<size_t> deserialize<Set<size_t>>(const Byte *data, size_t &p) {
    auto len = deserialize<size_t>(data, p);
    Set<size_t> objects{};
    objects.reserve(len);
    for (int i = 0; i < len; ++i) {
        objects.emplace(deserialize<size_t>(data, p));
    }
    return objects;
}

template<typename T>
std::unique_ptr<T> deserializePtr(const Byte *data, size_t &p) {
    auto b = deserialize<bool>(data, p);
    if (b) {
        return std::make_unique<T>(data, p);
    } else {
        return nullptr;
    }
}

template<typename T>
Bytes serializePtr(const std::unique_ptr<T> &val) {
    Bytes bytes(serialize(bool(val)));
    if (val) {
        bytes += val->toBytes();
    }
    return bytes;
}

/////////////////////////////////////////

template Bytes serializePrimitive<>(float);
template float deserializePrimitive<float>(const Byte *data, size_t &p);

template Bytes serializeMap<Str, Object>(const Map<Str, Object> &);
template Bytes serializeMap<Str, IndexNode::Ptr>(const Map<Str, IndexNode::Ptr> &);
template Bytes serializeMap<Str, Set<Str>>(const Map<Str, Set<Str>> &);
template Bytes serializeMap<Str, Set<size_t>>(const Map<Str, Set<size_t>> &);
template Bytes serializeMap<Str, Map<Str, Set<size_t>>>(const Map<Str, Map<Str, Set<size_t>>> &);
template Bytes serializeMap<Str, Map<Str, Set<Str>>>(const Map<Str, Map<Str, Set<Str>>> &);

template std::unique_ptr<KeyMapper> deserializePtr<KeyMapper>(const Byte *byte, size_t &p);
template Bytes serializePtr<KeyMapper>(const std::unique_ptr<KeyMapper>&);

template Map<Str, Object> deserializeMap<Str, Object>(const Byte *data, size_t &p);
template Map<Str, IndexNode::Ptr> deserializeMap<Str, IndexNode::Ptr>(const Byte *data, size_t &p);
template Map<Str, Set<size_t>> deserializeMap<Str, Set<size_t>>(const Byte *data, size_t &p);
template Map<Str, Set<Str>> deserializeMap<Str, Set<Str>>(const Byte *data, size_t &p);
template Map<Str, Str> deserializeMap<Str, Str>(const Byte *data, size_t &p);
template Map<Str, Map<Str, Set<size_t>>> deserializeMap<Str, Map<Str, Set<size_t>>>(const Byte *data, size_t &p);
template Map<Str, Map<Str, Set<Str>>> deserializeMap<Str, Map<Str, Set<Str>>>(const Byte *data, size_t &p);
