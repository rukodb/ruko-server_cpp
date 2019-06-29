#pragma once

#include "IndexableData.hpp"
#include "ObjData.hpp"

using DictMap = Map<Str, Object>;

struct DictData : public ObjData, public IndexableData {
    DictMap map;

    Str toString() const override;
    bool hasKey(const Str &key) const override;
    Object &operator[](const Str &key) override;
    const Object &operator[](const Str &key) const override;
    Vec<Entry> iter() override;
    Vec<ConstEntry> iter() const override;
    Object *add(const Str &key, const Object &obj) override;

    OBJ_DATA_FIELD_MV(DictData, map);
};
