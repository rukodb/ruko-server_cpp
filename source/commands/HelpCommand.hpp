#pragma once

#include "Command.hpp"

class HelpCommand : public Command {
public:
    HelpCommand(const Byte *data, size_t &p);
    HelpCommand(const char *&data);
    Str toString() override;
    CommandResult perform(RukoDb &db) override;

private:
    std::string option;
};
