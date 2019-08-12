#pragma once

#include "Command.hpp"

class GetCommand : public Command {
public:
    GetCommand(const Byte *data, size_t &p);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> keys, fields, exclude;
};
