#pragma once
#include <istream>
#include <ostream>
#include "IndexNode.hpp"

class Object;
class StarIndex;


Bytes withLen(const Bytes &bytes);
template<typename T>
T deserialize(const Byte *data, size_t &p);
template<typename T> Bytes serializePrimitive(T t);
template<typename T> T deserializePrimitive(void *data);
Bytes serialize(float f);
Bytes serialize(Byte b);
Bytes serialize(size_t n);
Bytes serialize(int n);
Bytes serialize(bool n);
Bytes serialize(void *);
template<> float deserialize<float>(const Byte *data, size_t &p);
template<> Byte deserialize<Byte>(const Byte *data, size_t &p);
template<> size_t deserialize<size_t>(const Byte *data, size_t &p);
template<> int deserialize<int>(const Byte *data, size_t &p);
template<> bool deserialize<bool>(const Byte *data, size_t &p);
template<> void *deserialize<void *>(const Byte *data, size_t &p);
template<typename K, typename V> Bytes serializeMap(const Map<K, V> &map);
template<typename K, typename V> Map<K, V> deserializeMap(const Byte *data, size_t &p);
Bytes serialize(const Vec<Str> &vec);
Bytes serialize(const Vec<size_t> &vec);
Bytes serialize(const Set<Str> &index);
Bytes serialize(const Set<size_t> &index);
Bytes serialize(const Map<Str, Object> &index);
Bytes serialize(const Map<Str, IndexNode::Ptr> &index);
Bytes serialize(const Map<Str, Set<Str>> &index);
Bytes serialize(const Map<Str, Str> &index);
Bytes serialize(const Map<Str, Set<size_t>> &index);
Bytes serialize(const Map<Str, Map<Str, Set<Str>>> &index);
Bytes serialize(const Map<Str, Map<Str, Str>> &index);
Bytes serialize(const Map<Str, Map<Str, Set<size_t>>> &index);
template<> Map<Str, Object> deserialize<Map<Str, Object>>(const Byte *data, size_t &p);
template<> Map<Str, IndexNode::Ptr> deserialize<Map<Str, IndexNode::Ptr>>(const Byte *data, size_t &p);
template<> Map<Str, Set<Str>> deserialize<Map<Str, Set<Str>>>(const Byte *data, size_t &p);
template<> Map<Str, Str> deserialize<Map<Str, Str>>(const Byte *data, size_t &p);
template<> Map<Str, Set<size_t>> deserialize<Map<Str, Set<size_t>>>(const Byte *data, size_t &p);
template<> Map<Str, Map<Str, Set<Str>>>
deserialize<Map<Str, Map<Str, Set<Str>>>>(const Byte *data, size_t &p);
template<> Map<Str, Map<Str, Str>>
deserialize<Map<Str, Map<Str, Str>>>(const Byte *data, size_t &p);
template<> Map<Str, Map<Str, Set<size_t>>>
deserialize<Map<Str, Map<Str, Set<size_t>>>>(const Byte *data, size_t &p);
Bytes serialize(const Object &obj);
Bytes serialize(const StarIndex &index);
template<> Object deserialize<Object>(const Byte *data, size_t &p);
template<> StarIndex deserialize<StarIndex>(const Byte *data, size_t &p);
Bytes serialize(const char *str);
Bytes serialize(const Str &str);
Bytes serialize(const IndexNode::Ptr &node);
template<> Str deserialize<Str>(const Byte *data, size_t &p);
template<> IndexNode::Ptr deserialize<IndexNode::Ptr>(const Byte *data, size_t &p);
Bytes serialize(const Vec<Object> &data);
Bytes serialize(const Vec<StarIndex> &data);
template<> Vec<Object> deserialize<Vec<Object>>(const Byte *data, size_t &p);
template<> Vec<StarIndex> deserialize<Vec<StarIndex>>(const Byte *data, size_t &p);
template<> Vec<Str> deserialize<Vec<Str>>(const Byte *data, size_t &p);
template<> Set<Str> deserialize<Set<Str>>(const Byte *data, size_t &p);
template<> Set<size_t> deserialize<Set<size_t>>(const Byte *data, size_t &p);

/////////////////////////////////////////////

template<typename T>
std::unique_ptr<T> deserializePtr(const Byte *data, size_t &p);

template<typename T>
Bytes serializePtr(const std::unique_ptr<T> &val);


