#include "LputCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

LputCommand::LputCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)),
        value(deserialize<Object>(data, p)) {}

Str LputCommand::toString() {
    return "LPUT " + join(keys, ".") + " " + value.toString();
}

CommandResult LputCommand::perform(RukoDb &db) {
    bool write = db.lput(keys, value);
    return {{}, write};
}