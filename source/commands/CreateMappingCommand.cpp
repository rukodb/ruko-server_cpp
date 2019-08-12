#include "CreateMappingCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

CreateMappingCommand::CreateMappingCommand(const Byte *data, size_t &p) :
        location(deserialize<Vec<Str>>(data, p)),
        filter(deserialize<Vec<Str>>(data, p)) {}

Str CreateMappingCommand::toString() {
    return "CREATE_MAPPING " + join(location, ".") + " " + join(filter, ".");
}

CommandResult CreateMappingCommand::perform(RukoDb &db) {
    bool write = db.createMapping(location, filter);
    return {{}, write};
}
