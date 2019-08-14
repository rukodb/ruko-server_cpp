#include "LputCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"
#include "parsing.hpp"

LputCommand::LputCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)),
        value(deserialize<Object>(data, p)) {}

LputCommand::LputCommand(const char *&data) :
        keys(parseLocation(data)),
        value(parseObject(data)) {}

Str LputCommand::toString() {
    return "LPUT " + join(keys, ".") + " " + value.toString();
}

CommandResult LputCommand::perform(RukoDb &db) {
    bool write = db.lput(keys, value);
    return {{}, write};
}