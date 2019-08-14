#pragma once

#include "Command.hpp"

class DeleteMappingCommand : public Command {
public:
    DeleteMappingCommand(const Byte *data, size_t &p);
    DeleteMappingCommand(const char *&data);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> location;
};
