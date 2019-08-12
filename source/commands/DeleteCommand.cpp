#include "DeleteCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

DeleteCommand::DeleteCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)) {}

Str DeleteCommand::toString() {
    return "DELETE " + join(keys, ".");
}

CommandResult DeleteCommand::perform(RukoDb &db) {
    bool write = db.del(keys);
    return {{}, write};
}