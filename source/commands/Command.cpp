#include <iostream>
#include "Command.hpp"
#include "serialization.hpp"
#include "RukoDb.hpp"
#include "GetCommand.hpp"
#include "SetCommand.hpp"
#include "DeleteCommand.hpp"
#include "DeclareCommand.hpp"
#include "LputCommand.hpp"
#include "CreateMappingCommand.hpp"
#include "DeleteMappingCommand.hpp"
#include "GetMappingsCommand.hpp"
#include "HelpCommand.hpp"
#include "parsing.hpp"

#include <cstdint>

constexpr std::uint32_t hsh(char const *s, std::size_t count) {
    return ((count ? hsh(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

constexpr std::uint32_t operator "" _hsh(char const *s, std::size_t count) {
    return hsh(s, count);
}

std::uint32_t hsh(const std::string &s) {
    return hsh(s.c_str(), s.size());
}

Command::Ptr Command::fromStream(const Byte *data, size_t &p) {
    auto command = deserialize<Byte>(data, p);
    switch (command) {
        case 1:
            return std::make_unique<GetCommand>(data, p);
        case 2:
            return std::make_unique<SetCommand>(data, p);
        case 3:
            return std::make_unique<DeleteCommand>(data, p);
        case 4:
            return std::make_unique<DeclareCommand>(data, p);
        case 5:
            return std::make_unique<LputCommand>(data, p);
        case 6:
            return std::make_unique<CreateMappingCommand>(data, p);
        case 7:
            return std::make_unique<DeleteMappingCommand>(data, p);
        case 8:
            return std::make_unique<GetMappingsCommand>(data, p);
        default:
            std::cerr << "Invalid command: " << int(command) << std::endl;
            return nullptr;
    }
}

Vec<Str> Command::getNames() {
    return {"GET", "SET", "DELETE", "DECLARE", "LPUT", "CREATE_MAPPING", "DELETE_MAPPING", "GET_MAPPINGS", "HELP"};
}

Command::Ptr Command::fromString(const char *&pos) {
    skipSpaces(pos);
    auto firstWord = parseWord(pos);
    std::transform(firstWord.begin(), firstWord.end(), firstWord.begin(), ::toupper);
    switch (hsh(firstWord)) {
        case "GET"_hsh:
            return std::make_unique<GetCommand>(pos);
        case "SET"_hsh:
            return std::make_unique<SetCommand>(pos);
        case "DELETE"_hsh:
            return std::make_unique<DeleteCommand>(pos);
        case "DECLARE"_hsh:
            return std::make_unique<DeclareCommand>(pos);
        case "LPUT"_hsh:
            return std::make_unique<LputCommand>(pos);
        case "CREATE_MAPPING"_hsh:
            return std::make_unique<CreateMappingCommand>(pos);
        case "DELETE_MAPPING"_hsh:
            return std::make_unique<DeleteMappingCommand>(pos);
        case "GET_MAPPINGS"_hsh:
            return std::make_unique<GetMappingsCommand>(pos);
        case "HELP"_hsh:
            return std::make_unique<HelpCommand>(pos);
        default:
            return {};
    }
}
