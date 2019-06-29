#include "Object.hpp"
#include "ObjData.hpp"


struct NullData : public ObjData {
    NullData() = default;
    NullData(const Byte *data, size_t &p);
    bool operator==(const NullData &other) const;
    Str toString() const override;

    TO_BYTES(::Bytes(), override);
};
