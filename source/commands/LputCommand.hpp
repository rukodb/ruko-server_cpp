#pragma once

#include "Command.hpp"

class LputCommand : public Command {
public:
    LputCommand(const Byte *data, size_t &p);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> keys;
    Object value;
};
