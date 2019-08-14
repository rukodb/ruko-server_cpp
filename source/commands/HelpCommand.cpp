#include "HelpCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"
#include "parsing.hpp"

HelpCommand::HelpCommand(const Byte *data, size_t &p) {}

HelpCommand::HelpCommand(const char *&data) :
        option(*data ? parseWord(data) : "") {}

Str HelpCommand::toString() {
    return "HELP " + option;
}

CommandResult HelpCommand::perform(RukoDb &db) {
    Str msg = "Available commands: ";
    for (auto &i : Command::getNames()) {
        msg += i + ", ";
    }
    msg.pop_back();
    msg.pop_back();
    return {Object(msg).toBytes()};
}