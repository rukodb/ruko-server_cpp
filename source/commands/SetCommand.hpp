#pragma once

#include "Command.hpp"

class SetCommand : public Command {
public:
    SetCommand(const Byte *data, size_t &p);
    SetCommand(const char *&data);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> keys;
    Object obj;
};
