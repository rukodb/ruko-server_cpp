#pragma once

#include "Object.hpp"
#include "ObjData.hpp"


struct StringData : public ObjData {
    Str val;

    static Str escaped(const Str &s);
    Str toString() const override;

    OBJ_DATA_FIELD_MV(StringData, val);
};