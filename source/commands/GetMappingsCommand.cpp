#include "GetMappingsCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

GetMappingsCommand::GetMappingsCommand(const Byte *data, size_t &p) {}

Str GetMappingsCommand::toString() {
    return "GET_MAPPINGS";
}

CommandResult GetMappingsCommand::perform(RukoDb &db) {
    return {db.getMappings().toBytes()};
}