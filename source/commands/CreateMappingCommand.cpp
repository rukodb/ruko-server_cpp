#include "CreateMappingCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"
#include "parsing.hpp"

CreateMappingCommand::CreateMappingCommand(const Byte *data, size_t &p) :
        location(deserialize<Vec<Str>>(data, p)),
        filter(deserialize<Vec<Str>>(data, p)) {}

CreateMappingCommand::CreateMappingCommand(const char *&data) :
        location(parseLocation(data)),
        filter(parseLocation(data)) {}

Str CreateMappingCommand::toString() {
    return "CREATE_MAPPING " + join(location, ".") + " " + join(filter, ".");
}

CommandResult CreateMappingCommand::perform(RukoDb &db) {
    bool write = db.createMapping(location, filter);
    return {{}, write};
}


