#pragma once

#include <memory>
#include "utils.hpp"
#include "KeyMapper.hpp"


struct IndexNode {
    using Ptr = std::unique_ptr<IndexNode>;
    Map<Str, IndexNode::Ptr> children;
    KeyMapper::Ptr mapper;

    IndexNode() = default;
    IndexNode(const Byte *data, size_t &p);
    Bytes toBytes() const;

    KeyMapper &getMapper() {
        if (!mapper) {
            mapper = std::make_unique<KeyMapper>();
        }
        return *mapper;
    }
};
