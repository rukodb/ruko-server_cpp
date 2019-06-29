#pragma once

#include "Object.hpp"
#include "ObjData.hpp"


struct IntData : public ObjData {
    int val{};

    Str toString() const override;

    OBJ_DATA_FIELD(IntData, val);
};
