#include "libs/catch.hpp"
#include "../source/RukoServer.hpp"
#include "../source/objects/DictData.hpp"

std::string objStr(const Object &obj) {
    std::string str = obj.toString();
    std::replace(str.begin(), str.end(), '"', '\'');
    return str;
}

TEST_CASE("ruko") {
    RukoDb db;
    SECTION("empty") {
        auto obj = db.get({});
        REQUIRE(obj.getId() == DICT_ID);
        CHECK(obj.get<DictData>().map.empty());
    }
    SECTION("Nonexistent") {
        auto res = db.get({"key"});
        CHECK(db.get({"key"}) == Object());
    }
    SECTION("set get root") {
        db.set({"val"}, Object("hello"));
        db.set({}, Object(DICT_ID));
        CHECK(db.get({}) == Object(DICT_ID));
        db.set({}, Object("hi"));
        CHECK(db.get({}) == Object("hi"));
    }
    SECTION("get nonexistent") {
        CHECK(db.get({"random"}) == Object());
    }
    SECTION("set get level height 1") {
        CHECK(db.set({"val"}, Object("hello")));
        auto wholeDb = db.get({});
        CHECK(wholeDb.get<DictData>().map == Map<std::string, Object>({{"val", Object("hello")}}));
        CHECK(db.get({"val"}) == Object("hello"));
    }
    SECTION("set get height multiple") {
        CHECK(db.set({"key1", "key2"}, Object("value")));
        CHECK(objStr(db.get({})) == "{'key1': {'key2': 'value'}}");
        CHECK(db.get({"key1", "key2"}) == Object("value"));
    }
    SECTION("get list index") {
        db.set({"list"}, Object(Vec<Object>({"zero", "one", "two"})));
        CHECK(db.get({"list", "0"}) == Object("zero"));
        CHECK(db.get({"list", "2"}) == Object("two"));
        CHECK(db.get({"list", "-1"}) == Object("two"));
        CHECK(db.get({"list", "3"}) == Object());
        db.set({"list", "2"}, Object(12));
        CHECK(db.get({"list", "2"}) == Object(12));
    }
    SECTION("delete") {
        SECTION("by assignment") {
            db.set({"users"}, Object("a"));
            db.set({"devices"}, Object("b"));
            db.set({}, Object(DICT_ID));
            CHECK(db.get({}) == Object(DICT_ID));
            CHECK(db.get({"users"}) == Object());
            CHECK(db.get({"devices"}) == Object());
        }
        SECTION("from dict") {
            db.set({"dict", "key"}, "hello");
            db.del({"dict", "key"});
            CHECK(db.get({"dict", "key"}) == Object());
        }
        SECTION("from list") {
            db.set({"list"}, Object(LIST_ID));
            REQUIRE(db.lput({"list"}, Object("first")));
            REQUIRE(db.lput({"list"}, Object("second")));
            REQUIRE(db.get({"list", "0"}) == Object("first"));
            db.del({"list", "0"});
            CHECK(db.get({"list", "0"}) == Object("second"));
        }
        SECTION("mapping") {
            db.createMapping({"mapping"}, {"things", "*"});
            db.set({"things", "abc"}, Object::parse(R"({"key1": "val1", "key2": "val2"})"));
            REQUIRE(db.get({"mapping", "key1:val1"}) == Object::parse(R"({"key1": "val1", "key2": "val2"})"));
            db.del({});
            CHECK(db.get({}) == Object(DICT_ID));
        }
        SECTION("mapping at pos") {
            SECTION("dict") {
                db.createMapping({"mapping"}, {"things", "*"});
                db.set({"things", "abc"}, Object::parse(R"({"key1": "val1", "key2": "val2"})"));
                REQUIRE(db.get({"mapping", "key1:val1"}) == Object::parse(R"({"key1": "val1", "key2": "val2"})"));
                db.del({"mapping", "key1:val1"});
                CHECK(db.get({}) == Object::parse(R"({"things": {}})"));
            }
            SECTION("list") {
                db.createMapping({"mapping"}, {"things", "*"});
                db.lput({"things"}, Object::parse(R"({"key1": "val1", "key2": "val2"})"));
                REQUIRE(db.get({"mapping", "key1:val1"}) == Object::parse(R"({"key1": "val1", "key2": "val2"})"));
                db.del({"mapping", "key1:val1"});
                CHECK(db.get({}) == Object::parse(R"({"things": []})"));
            }
        }
    }
    SECTION("declare") {
        SECTION("dict") {
            db.declare({"key"}, DICT_ID);
            CHECK(db.get({"key"}) == Object(DICT_ID));
        }
        SECTION("list") {
            db.declare({"key"}, LIST_ID);
            CHECK(db.get({"key"}) == Object(LIST_ID));
        }
        SECTION("existing") {
            SECTION("other type") {
                db.set({"key"}, "hello");
                db.declare({"key"}, LIST_ID);
                CHECK(db.get({"key"}) == Object(LIST_ID));
            }
            SECTION("same type") {
                db.set({"key"}, "hello");
                db.declare({"key"}, STRING_ID);
                CHECK(db.get({"key"}) == Object("hello"));
            }
        }
    }
    SECTION("lput") {
        db.declare({"users"}, LIST_ID);
        db.lput({"users"}, Object("joe"));
        CHECK(db.get({"users", "0"}) == Object("joe"));
    }
}

TEST_CASE("ruko-indices") {
    RukoDb db, db2;

    SECTION("declare") {
        db.set({"users"}, Object::parse(R"([{"name": "john"}, {"name": "joe"}])"));

        db.declare({"users"}, LIST_ID, {"name"});
        const auto &val = db.get({"users", "name:john"});
        CHECK(objStr(val) == "{'name': 'john'}");
    }
    SECTION("get by", "[.]") {
        SECTION("list") {
            db.set({"users"}, Vec<Object>({DictMap({{"name", {"john"}}}),
                                           DictMap({{"name", {"joe"}}})}));
            CHECK(objStr(db.get({"users", "name:john"})) == "{'name': 'john'}");
        }
        SECTION("dict") {
            db.set({"users"}, DictMap({{"abc", DictMap({{"name", {"john"}}})},
                                       {"def", DictMap({{"name", {"joe"}}})}}));
            CHECK(objStr(db.get({"users", "name:john"})) == "{'name': 'john'}");
        }
        SECTION("updating indices") {
            SECTION("list") {
                db.declare({"key"}, LIST_ID, {"name"});
                REQUIRE(db.lput({"key"}, Object(DictMap({{"name", "john"}}))));
                REQUIRE(db.lput({"key"}, Object(DictMap({{"name", "joe"}}))));
                CHECK(db.get({"key", "name:john"}) == Object(DictMap({{"name", "john"}})));
                db.del({"key", "0"});
                CHECK(db.get({"key", "name:john"}) == Object());
                CHECK(db.get({"key", "name:joe"}) == Object(DictMap({{"name", "joe"}})));
            }
            SECTION("dict") {
                db.declare({"key"}, DICT_ID, {"name"});
                db.set({"key", "abc123"}, Object(DictMap({{"name", "john"}})));
                db.set({"key", "def234"}, Object(DictMap({{"name", "joe"}})));
                CHECK(db.get({"key", "name:john"}) == Object(DictMap({{"name", "john"}})));
                db.del({"key", "abc123"});
                CHECK(db.get({"key", "name:john"}) == Object());
                CHECK(db.get({"key", "name:joe"}) == Object(DictMap({{"name", "joe"}})));
            }
        }
        SECTION("at end") {
            db.set({}, Object::parse(R"({"a": [{"uuid": "a0", "data": "d0"}, {"uuid": "a1", "data": "d1"}]})"));
            CHECK(db.get({"a", "uuid:"}) ==
                  Object::parse(R"({"a0": {"uuid": "a0", "data": "d0"}, "a1": {"uuid": "a1", "data": "d1"}})"));
        }
        SECTION("empty overwrite") {
            db.get({"users", "email:abc@abc.com"});
            db.set({"users"}, Object(LIST_ID));
            REQUIRE(db.get({}) == Object::parse(R"({"users": []})"));
        }
    }
    SECTION("mapping") {
        db.set({"users"}, Object::parse(R"([{"uuid": "u0u", "devices": [{"uuid": "u0d0u"}, {"uuid": "u0d1u"}]},
                                            {"uuid": "u1u", "devices": [{"uuid": "u1d0u"}, {"uuid": "u1d1u"}]}])"));
        db.createMapping({"objects"}, {"users", "*"});
        db.createMapping({"objects"}, {"users", "*", "devices", "*"});
        CHECK(db.get({"objects", "uuid:u0u", "uuid"}) == "u0u");
        CHECK(db.get({"objects", "uuid:u0d0u", "uuid"}) == "u0d0u");
        CHECK(db.get({"objects", "uuid:u0d1u", "uuid"}) == "u0d1u");
        CHECK(db.get({"objects", "uuid:u1u", "uuid"}) == "u1u");
        CHECK(db.get({"objects", "uuid:u1d0u", "uuid"}) == "u1d0u");
        CHECK(db.get({"objects", "uuid:u1d1u", "uuid"}) == "u1d1u");
    }
    SECTION("mapping update") {
        db.set({"users"}, Object::parse(R"([{"uuid": "u0u", "devices": [{"uuid": "u0d0u"}, {"uuid": "u0d1u"}]}])"));
        db.createMapping({"objects"}, {"users", "*"});
        db.createMapping({"objects"}, {"users", "*", "devices", "*"});
        REQUIRE(db.get({"objects", "uuid:u1u"}) == Object());
        db.lput({"users"}, Object::parse(R"({"uuid": "u1u", "devices": [{"uuid": "u1d0u"}, {"uuid": "u1d1u"}]})"));
        CHECK(db.get({"objects", "uuid:u0u", "uuid"}) == "u0u");
        CHECK(db.get({"objects", "uuid:u0d0u", "uuid"}) == "u0d0u");
        CHECK(db.get({"objects", "uuid:u0d1u", "uuid"}) == "u0d1u");
        CHECK(db.get({"objects", "uuid:u1u", "uuid"}) == "u1u");
        CHECK(db.get({"objects", "uuid:u1d0u", "uuid"}) == "u1d0u");
        CHECK(db.get({"objects", "uuid:u1d1u", "uuid"}) == "u1d1u");
    }
    SECTION("mapping-2") {
        db.set({"users"}, Object::parse(R"([])"));
        db.createMapping({"auths"}, {"users", "*", "auths", "*"});
        db.createMapping({"auths"}, {"users", "*", "devices", "*", "auths", "*"});
        REQUIRE(db.get({"auths", "token:u0u"}) == Object());
        db.lput({"users"}, Object::parse(R"({"auths": [{"token": "u0u"}], "devices": []})"));
        db.lput({"users", "0", "devices"}, Object::parse(R"({"auths": [{"token": "u0d0u"}]})"));
        db.lput({"users", "0", "devices"}, Object::parse(R"({"auths": [{"token": "u0d1u"}]}]})"));
        db.lput({"users"}, Object::parse(R"({"auths": [{"token": "u1u"}], "devices": []})"));
        db.lput({"users", "1", "devices"}, Object::parse(R"({"auths": [{"token": "u1d0u"}]})"));
        db.lput({"users", "1", "devices"}, Object::parse(R"({"auths": [{"token": "u1d1u"}]}]})"));

        CHECK(db.get({"auths", "token:u0u", "token"}) == "u0u");
        CHECK(db.get({"auths", "token:u0d0u", "token"}) == "u0d0u");
        CHECK(db.get({"auths", "token:u0d1u", "token"}) == "u0d1u");
        CHECK(db.get({"auths", "token:u1u", "token"}) == "u1u");
        CHECK(db.get({"auths", "token:u1d0u", "token"}) == "u1d0u");
        CHECK(db.get({"auths", "token:u1d1u", "token"}) == "u1d1u");
    }
    SECTION("mapping update 1") {
        db.set({"users"}, Object::parse(R"([{"uuid": "u0u", "devices": [{"uuid": "u0d0u"}, {"uuid": "u0d1u"}]}])"));
        db.createMapping({"objects"}, {"users", "*"});
        db.createMapping({"objects"}, {"users", "*", "devices", "*"});
        REQUIRE(db.get({"objects", "uuid:u1u"}) == Object());
        db.lput({"users"}, Object::parse(R"({"uuid": "u1u", "devices": [{"uuid": "u1d0u"}, {"uuid": "u1d1u"}]})"));
        CHECK(db.get({"objects", "uuid:u0u", "uuid"}) == "u0u");
        CHECK(db.get({"objects", "uuid:u0d0u", "uuid"}) == "u0d0u");
        CHECK(db.get({"objects", "uuid:u0d1u", "uuid"}) == "u0d1u");
        CHECK(db.get({"objects", "uuid:u1u", "uuid"}) == "u1u");
        CHECK(db.get({"objects", "uuid:u1d0u", "uuid"}) == "u1d0u");
        CHECK(db.get({"objects", "uuid:u1d1u", "uuid"}) == "u1d1u");
    }
    SECTION("mapping update 2") {
        db.set({"users"}, Object::parse(R"([])"));
        db.createMapping({"devices"}, {"users", "*", "devices", "*"});
        REQUIRE(db.get({"devices", "uuid:u0d0"}) == Object());
        db.lput({"users"}, Object::parse(R"({"uuid": "u0u", "devices": []})"));
        REQUIRE(db.get({"users", "uuid:u0u", "uuid"}) == "u0u");
        db.lput({"users", "uuid:u0u", "devices"}, Object::parse(R"({"uuid": "u0d0u"})"));
        CHECK(db.get({"devices", "uuid:u0d0u", "uuid"}) == "u0d0u");
    }
    SECTION("mapping write") {
        db.createMapping({"mapping"}, {"things", "*"});
        db.set({"things", "abc"}, Object::parse(R"({"key1": "val1", "key2": "val2"})"));
        db.set({"mapping", "key1:val1", "key2"}, Object::parse(R"("newVal2")"));
        REQUIRE(db.get({"things", "abc", "key2"}) == "newVal2");
    }
    SECTION("saving") {
        SECTION("simple") {
            std::remove("tmp.db");
            RukoDb db("tmp.db");
            db.set({}, Object::parse(R"({"a": [{"uuid": "a0", "data": "d0"}, {"uuid": "a1", "data": "d1"}]})"));
            db.save();
            RukoDb db2("tmp.db");
            REQUIRE(db2.get({}) ==
                    Object::parse(R"({"a": [{"uuid": "a0", "data": "d0"}, {"uuid": "a1", "data": "d1"}]})"));
        }
        SECTION("complex") {
            std::remove("tmp.db");
            RukoDb db("tmp.db");
            db.set({}, Object::parse(R"({"a": [{"uuid": "a0", "data": "d0"}, {"uuid": "a1", "data": "d1"}]})"));
            db.createMapping({"things"}, {"a", "*"});
            REQUIRE(db.get({"things", "uuid:a0", "uuid"}) == "a0");
            db.save();
            RukoDb db2("tmp.db");
            REQUIRE(db2.get({}) ==
                    Object::parse(R"({"a": [{"uuid": "a0", "data": "d0"}, {"uuid": "a1", "data": "d1"}]})"));
            REQUIRE(db2.get({"things", "uuid:a0", "uuid"}) == "a0");
        }
    }
}

TEST_CASE("debug") {
    RukoDb db;
    SECTION("mapping update 3") {
        db.get({"users"});
        db.set({"users"}, Object::parse(R"([])"));
        db.createMapping({"devices"}, {"users", "*", "devices", "*"});
        db.lput({"users"}, Object::parse(R"({"uuid": "u0u", "devices": []})"));
        db.lput({"users"}, Object::parse(R"({"uuid": "u1u", "devices": []})"));
        db.get({"users", "uuid:u0u", "uuid"});
        db.lput({"users"}, Object::parse(R"({"uuid": "u2u", "devices": []})"));
        db.get({"devices", "uuid:u0d0u"});
        db.lput({"users", "uuid:u0u", "devices"}, Object::parse(R"({"uuid": "u0d0u"})"));
        CHECK(db.get({"devices", "uuid:u0d0u", "uuid"}) == "u0d0u");
    }
}

