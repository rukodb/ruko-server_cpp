#pragma once

#include "ObjData.hpp"
#include "IndexableData.hpp"


struct ListData : public ObjData, public IndexableData {
    Vec<Object> list;
    Str toString() const override;
    std::pair<bool, int> parseIndex(const Str &key) const;
    bool hasKey(const Str &key) const override;
    Object &operator[](const Str &key) override;
    const Object &operator[](const Str &key) const override;
    Vec<Entry> iter() override;
    Vec<ConstEntry> iter() const override;
    Object *add(const Str &key, const Object &obj) override;

    OBJ_DATA_FIELD_MV(ListData, list);
};
