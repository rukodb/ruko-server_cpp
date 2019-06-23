#include "libs/catch.hpp"
#include "../source/RukoServer.hpp"

using Str = std::string;

TEST_CASE("index-manager") {
    IndexManager manager;

    REQUIRE(manager.convertKeys({}, Object::parse(R"({})")).empty());
    REQUIRE(manager.convertKeys({"one"}, Object::parse(R"({})")) == Vec<Str>({"one"}));
    REQUIRE(manager.convertKeys({"one", "two"}, Object::parse(R"({})")) == Vec<Str>({"one", "two"}));

    Object db = Object::parse(R"({"users": {"abc": {"name": "john", "data": 123}}})");

    manager.handleCreate({"users", "abc"}, db);
    SECTION("simple") {
        CHECK(manager.convertKeys({"users", "name:john"}, db) == Vec<Str>{"users", "abc"});
        CHECK(manager.convertKeys({"users", "name:blah"}, db) == Vec<Str>{"users", ""});
        CHECK(manager.convertKeys({"users", "name:"}, db) == Vec<Str>{"users", "name:"});
    }
    SECTION("parent") {
        CHECK(manager.convertKeys({"users", "---"}, db) == Vec<Str>{});
        CHECK(manager.convertKeys({"users", "---"}, db) == Vec<Str>{});
        CHECK(manager.convertKeys({"---"}, db) == Vec<Str>{});
        CHECK(manager.convertKeys({"users", "---", "val"}, db) == Vec<Str>{"val"});
        CHECK(manager.convertKeys({"---", "val"}, db) == Vec<Str>{"val"});
        CHECK(manager.convertKeys({"---", "val"}, db) == Vec<Str>{"val"});
        CHECK(manager.convertKeys({"users", "name:john", "---"}, db) == Vec<Str>{"users"});
    }
    SECTION("mappings") {
        SECTION("dict") {
            SECTION("simple") {
                db = Object::parse(R"(
                    {
                        "users": {
                            "abc": {"name": "john", "data": 123, "devices": {"1": {"uuid": "a"}, "2": {"uuid": "b"}}},
                            "def": {"name": "rob", "data": 456, "devices": {"3": {"uuid": "c"}, "4": {"uuid": "d"}}}
                        }
                    }
                )");
                manager.createMapping({"devices"}, {"users", "*", "devices", "*"}, db);
                CHECK(manager.convertKeys({"devices", "uuid:a"}, db) == Vec<Str>{"users", "abc", "devices", "1"});
                CHECK(manager.convertKeys({"devices", "uuid:b"}, db) == Vec<Str>{"users", "abc", "devices", "2"});
                CHECK(manager.convertKeys({"devices", "uuid:c"}, db) == Vec<Str>{"users", "def", "devices", "3"});
                CHECK(manager.convertKeys({"devices", "uuid:d"}, db) == Vec<Str>{"users", "def", "devices", "4"});
            }
            SECTION("duplicate valid") {
                db = Object::parse(R"(
                    {
                        "users": {
                            "abc": {"name": "john", "data": 123, "devices": {"1": {"uuid": "a"}, "2": {"uuid": "b"}}},
                            "def": {"name": "rob", "data": 456, "devices": {"1": {"uuid": "c"}, "2": {"uuid": "d"}}}
                        }
                    }
                )");
                manager.createMapping({"devices"}, {"users", "*", "devices", "*"}, db);
                CHECK(manager.convertKeys({"devices", "uuid:a"}, db) == Vec<Str>{"users", "abc", "devices", "1"});
                CHECK(manager.convertKeys({"devices", "uuid:b"}, db) == Vec<Str>{"users", "abc", "devices", "2"});
                CHECK(manager.convertKeys({"devices", "uuid:c"}, db) == Vec<Str>{"users", "def", "devices", "1"});
                CHECK(manager.convertKeys({"devices", "uuid:d"}, db) == Vec<Str>{"users", "def", "devices", "2"});
            }
            SECTION("delete") {
                db = Object::parse(R"(
                    {
                        "users": {
                            "abc": {"name": "john", "data": 123, "devices": {"1": {"uuid": "a"}, "2": {"uuid": "b"}}},
                            "def": {"name": "rob", "data": 456, "devices": {"3": {"uuid": "c"}, "4": {"uuid": "d"}}}
                        }
                    }
                )");
                manager.createMapping({"devices"}, {"users", "*", "devices", "*"}, db);
                REQUIRE(manager.deleteMapping({"devices"}, db));
                CHECK(!manager.deleteMapping({"devices"}, db));
                CHECK(manager.convertKeys({"devices", "uuid:a"}, db) == Vec<Str>{"devices", ""});
                CHECK(manager.convertKeys({"devices", "uuid:b"}, db) == Vec<Str>{"devices", ""});
                CHECK(!IndexManager::isValid(manager.convertKeys({"devices", "uuid:c"}, db)));
                CHECK(!IndexManager::isValid(manager.convertKeys({"devices", "uuid:d"}, db)));
            }
        }
        SECTION("list") {
            SECTION("simple") {
                db = Object::parse(R"(
                    {
                        "users": {
                            "abc": {"name": "john", "data": 123, "devices": [{"uuid": "a"}, {"uuid": "b"}]},
                            "def": {"name": "rob", "data": 456, "devices": [{"uuid": "c"}, {"uuid": "d"}]}
                        }
                    }
                )");
                manager.createMapping({"devices"}, {"users", "*", "devices", "*"}, db);
                CHECK(manager.convertKeys({"devices", "uuid:a"}, db) == Vec<Str>{"users", "abc", "devices", "0"});
                CHECK(manager.convertKeys({"devices", "uuid:b"}, db) == Vec<Str>{"users", "abc", "devices", "1"});
                CHECK(manager.convertKeys({"devices", "uuid:c"}, db) == Vec<Str>{"users", "def", "devices", "0"});
                CHECK(manager.convertKeys({"devices", "uuid:d"}, db) == Vec<Str>{"users", "def", "devices", "1"});
            }
        }
    }
    SECTION("handleDelete handleCreate") {
        db = Object::parse(R"(
            {
                "users": "a",
                "devices": "b"
            }
        )");
        manager.handleCreate({"users"}, db);
        manager.handleCreate({"devices"}, db);
        manager.handleDelete({}, db);
    }
    SECTION("multiple") {
        IndexManager manager;
        auto db = Object::parse(R"(
            {
                "users": "a",
                "devices": "b"
            }
        )");
        manager.handleCreate({"users"}, db);
        manager.handleCreate({"devices"}, db);
        manager.handleDelete({}, db);
    }
    SECTION("mappings") {
        db = Object::parse(R"(
            {
                "a": [{"uuid": "a0"}, {"uuid": "a1"}],
                "b": [{"uuid": "b0"}, {"uuid": "b1"}]
            }
        )");
        SECTION("single") {
            manager.createMapping({"auths"}, {"a", "*"}, db);
            CHECK(manager.convertKeys({"auths", "uuid:a0"}, db) == Vec<Str>({"a", "0"}));
            CHECK(manager.convertKeys({"auths", "uuid:a1"}, db) == Vec<Str>({"a", "1"}));
        }
        SECTION("multiple") {
            manager.createMapping({"auths"}, {"a", "*"}, db);
            manager.createMapping({"auths"}, {"b", "*"}, db);
            CHECK(manager.convertKeys({"auths", "uuid:a0"}, db) == Vec<Str>({"a", "0"}));
            CHECK(manager.convertKeys({"auths", "uuid:a1"}, db) == Vec<Str>({"a", "1"}));
            CHECK(manager.convertKeys({"auths", "uuid:b0"}, db) == Vec<Str>({"b", "0"}));
            CHECK(manager.convertKeys({"auths", "uuid:b1"}, db) == Vec<Str>({"b", "1"}));
        }
    }
}
