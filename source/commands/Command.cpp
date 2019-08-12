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

