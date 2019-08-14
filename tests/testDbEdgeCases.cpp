#include <RukoDb.hpp>
#include <commands/Command.hpp>
#include "libs/catch.hpp"
#include <iostream>

using RukoDbPtr = std::unique_ptr<RukoDb>;


RukoDbPtr runCommands(const Str &commands) {
    auto db = std::make_unique<RukoDb>();
    auto commandsa = split(commands, '\n');
    for (auto &c : commandsa) {
        auto *pos = c.c_str();
        auto cmd = Command::fromString(pos);
        if (cmd) {
            cmd->perform(*db);
        }
    }
    return {};
}

TEST_CASE("Double new index") {
    runCommands(R"(
CREATE_MAPPING auths users.*.auths.*
LPUT users {"auths": [{"refresh": "abc", "token": "def"}], "devices": []}
GET auths.token:hji
GET auths.refresh:kjl
)");
}
