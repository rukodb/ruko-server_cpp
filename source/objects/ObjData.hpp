#pragma once

#include <utils.hpp>

struct ObjData {
    virtual Bytes toBytes() const = 0;
    virtual Str toString() const = 0;
    virtual ~ObjData() = default;
};
