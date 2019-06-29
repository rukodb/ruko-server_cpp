#pragma once

#include "Object.hpp"
#include "ObjData.hpp"


struct FloatData : public ObjData {
    float val{};

    Str toString() const override;

    OBJ_DATA_FIELD(FloatData, val);
};