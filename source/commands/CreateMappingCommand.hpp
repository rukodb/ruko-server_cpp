#pragma once

#include "Command.hpp"

class CreateMappingCommand : public Command {
public:
    CreateMappingCommand(const Byte *data, size_t &p);
    CreateMappingCommand(const char *&data);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    Vec<Str> location, filter;
};
