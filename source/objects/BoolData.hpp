#include "Object.hpp"
#include "ObjData.hpp"

struct BoolData : public ObjData {
    bool val{};

    Str toString() const override;

    OBJ_DATA_FIELD(BoolData, val);
};
