#pragma once

#include <vector>
#include <cstring>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <istream>
#include <ostream>
#include <sstream>

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)
#define eassert(expr) if (!(expr)) { throw std::runtime_error(LOCATION ": Assert " #expr); }

template<typename K, typename V>
using Map = std::unordered_map<K, V>;
template<typename T>
using Set = std::unordered_set<T>;
template<typename T>
using Vec = std::vector<T>;
using Byte = unsigned char;
using Bytes = Vec<Byte>;
using Str = std::string;

Bytes operator+(const Bytes &a, const Bytes &b);
Bytes &operator+=(Bytes &a, const Bytes &b);

template<typename T>
const T *bytesToPointer(const Bytes &bytes) {
    static_assert(sizeof(T) == sizeof(Byte), "Casting incompatible pointer");
    return reinterpret_cast<const T *>(bytes.data());
}

/// Interpret 4 bytes in little endian as a u32 number
template<typename T>
T _bytesToP32(const Byte *data, size_t &p) {
    T d = T(data[p]) +
          T(data[p + 1] << 8) +
          T(data[p + 2] << 16) +
          T(data[p + 3] << 24);
    p += 4;
    return d;
}

template<typename T>
Bytes _p32ToBytes(T n) {
    return {
            (Byte) (n),
            (Byte) (n >> 8),
            (Byte) (n >> 16),
            (Byte) (n >> 24)
    };
}

extern uint32_t (*bytesToU32)(const Byte *, size_t &);
extern int32_t (*bytesToI32)(const Byte *, size_t &);
extern Bytes (*u32ToBytes)(uint32_t);
extern Bytes (*i32ToBytes)(int32_t);

template<typename...Args, typename T>
T concatArgs(T a, T b, Args...args) {
    return concatArgs(a + b, args...);
}

template<typename T>
T concatArgs(const T &a, const T &b) {
    return a + b;
}

template<typename T>
T concatArgs(const T &a) {
    return a;
}

template<typename...Args, typename T>
T concat(T a, T b, Args...args) {
    return concatArgs(a + b, args...);
}

template<typename T, typename V>
T concat(const V &vec) {
    T out;
    for (auto &i : vec) {
        out = out + i;
    }
    return out;
}

template<typename T>
T concat(const Vec<T> &vec) {
    return concat<T, Vec<T>>(vec);
}

Str join(const Vec<Str> &parts, const Str &sep);