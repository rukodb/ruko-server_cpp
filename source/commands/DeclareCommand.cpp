#include "DeclareCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"

DeclareCommand::DeclareCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)),
        dataType(deserialize<Byte>(data, p)),
        indices(deserialize<Vec<Str>>(data, p)) {}

Str DeclareCommand::toString() {
    return "DECLARE " + join(keys, ".") + " " + std::__cxx11::to_string(dataType) +
           (indices.empty() ? "" : " " + join(indices, " '))"));
}

CommandResult DeclareCommand::perform(RukoDb &db) {
    bool write = db.declare(keys, dataType, indices);
    return {{}, write};
}