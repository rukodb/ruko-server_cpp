#include "GetCommand.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"
#include "parsing.hpp"

GetCommand::GetCommand(const Byte *data, size_t &p) :
        keys(deserialize<Vec<Str>>(data, p)),
        fields(deserialize<Vec<Str>>(data, p)),
        exclude(deserialize<Vec<Str>>(data, p)) {}

GetCommand::GetCommand(const char *&data) :
        keys(parseLocation(data)),
        fields(parseWordTest(data, "FIELDS") ? parseList(data) : Vec<Str>()),
        exclude(parseWordTest(data, "EXCLUDE") ? parseList(data) : Vec<Str>()) {}

Str GetCommand::toString() {
    return "GET " + join(keys, ".") +
           (fields.empty() ? "" : " FIELDS " + join(fields, ",")) +
           (exclude.empty() ? "" : " EXCLUDE " + join(exclude, ","));
}

CommandResult GetCommand::perform(RukoDb &db) {
    auto res = db.get(keys, fields, exclude);
    if (res.isEmpty()) {
        return {{}};
    }
    return {res.toBytes()};
}