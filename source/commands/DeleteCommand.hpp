#pragma once

#include "Command.hpp"

class DeleteCommand : public Command {
public:
    DeleteCommand(const Byte *data, size_t &p);
    DeleteCommand(const char *&data);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> keys;
};
