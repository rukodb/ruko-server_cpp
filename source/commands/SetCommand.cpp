#include "SetCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

SetCommand::SetCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)),
        obj(deserialize<Object>(data, p)) {}

Str SetCommand::toString() {
    return "SET " + join(keys, ".") + " " + obj.toString();
}

CommandResult SetCommand::perform(RukoDb &db) {
    bool write = db.set(keys, obj);
    return {{}, write};
}