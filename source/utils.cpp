#include <cstdint>
#include "utils.hpp"

Bytes operator+(const Bytes &a, const Bytes &b) {
    Bytes c;
    c.insert(c.end(), a.begin(), a.end());
    c.insert(c.end(), b.begin(), b.end());
    return std::move(c);
}

Bytes &operator+=(Bytes &a, const Bytes &b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

uint32_t (*bytesToU32)(const Byte *, size_t &) = &_bytesToP32<uint32_t>;
int32_t (*bytesToI32)(const Byte *, size_t &) = &_bytesToP32<int32_t>;
Bytes (*u32ToBytes)(uint32_t) = &_p32ToBytes<uint32_t>;
Bytes (*i32ToBytes)(int32_t) = &_p32ToBytes<int32_t>;



