#include "DeleteMappingCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

DeleteMappingCommand::DeleteMappingCommand(const Byte *data, size_t &p) :
        location(deserialize<Vec<Str>>(data, p)) {}

Str DeleteMappingCommand::toString() {
    return "DELETE_MAPPING " + join(location, ".");
}

CommandResult DeleteMappingCommand::perform(RukoDb &db) {
    bool write = db.deleteMapping(location);
    return {{}, write};
}