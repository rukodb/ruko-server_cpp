#pragma once

#include <memory>
#include <string>
#include <objects/Object.hpp>
#include "utils.hpp"


class RukoDb;

struct CommandResult {
    Bytes output;
    bool didWrite = false;
};

class Command {
public:
    using Ptr = std::unique_ptr<Command>;

    static Ptr fromStream(const Byte *data, size_t &p);

    virtual std::string toString() = 0;
    virtual CommandResult perform(RukoDb &db) = 0;

private:
    int id;
};

