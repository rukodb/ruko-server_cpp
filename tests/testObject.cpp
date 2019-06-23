#include "libs/catch.hpp"
#include "../source/RukoServer.hpp"


TEST_CASE("Object") {
    SECTION("equality") {
        CHECK(Object("hello") != Object("hi"));
        CHECK(Object("hello") == Object("hello"));
        CHECK(Object() != Object(DICT_ID));
        CHECK(Object(12) != Object(12.2f));
        CHECK(Object(12) != Object(true));
        CHECK(Object(13) == Object(13));
        CHECK(Object(false) == Object(false));
        CHECK(Object(nullptr) == Object(nullptr));
        CHECK(Object() == Object());
        CHECK(Object(nullptr) != Object());
        CHECK(Object(DictMap({{"key", {"value"}}})) == Object(DictMap({{"key", {"value"}}})));
    }
    SECTION("toString") {
        CHECK(Object("hello").toString() == R"("hello")");
        CHECK(Object(12).toString() == "12");
        CHECK(Object(12.4f).toString() == "12.4");
        CHECK(Object(true).toString() == "true");
        CHECK(Object(nullptr).toString() == "null");
        CHECK(Object().toString() == "?");
        CHECK(Object(DictMap({{"key", {"value"}}})).toString() == R"({"key": "value"})");
        CHECK(Object(DictMap()).toString() == "{}");
        CHECK(Object(Vec<Object>({"val1", "val2"})).toString() == R"(["val1", "val2"])");
        CHECK(Object(Vec<Object>()).toString() == "[]");
    }
    SECTION("parse") {
        SECTION("fundamental") {
            CHECK(Object::parse(R"("")").toString() == R"("")");
            CHECK(Object::parse(R"("hello")").toString() == R"("hello")");
            CHECK(Object::parse("0").toString() == "0");
            CHECK(Object::parse("12").toString() == "12");
            CHECK(Object::parse("12.2").get<FloatData>().val == Approx(12.2));
            CHECK(Object::parse(".3").get<FloatData>().val == Approx(0.3));
            CHECK(Object::parse("[]").toString() == "[]");
            CHECK(Object::parse("[1, 2, 3]").toString() == "[1, 2, 3]");
            CHECK(Object::parse(R"([1, "hi", 3])").toString() == R"([1, "hi", 3])");
            CHECK(Object::parse("{}").toString() == "{}");
            CHECK(Object::parse(R"({"key": 14})").toString() == R"({"key": 14})");
            CHECK(Object::parse(R"({"key": "value"})").toString() == R"({"key": "value"})");
            CHECK(Object::parse(R"({"key": {"key2": "value"}})").toString() == R"({"key": {"key2": "value"}})");
        }
        SECTION("spacing") {
            CHECK(Object::parse(R"(  ""  )").toString() == R"("")");
            CHECK(Object::parse(R"(  "hello"  )").toString() == R"("hello")");
            CHECK(Object::parse("  0  ").toString() == "0");
            CHECK(Object::parse("  12  ").toString() == "12");
            CHECK(Object::parse("  12.2  ").get<FloatData>().val == Approx(12.2));
            CHECK(Object::parse("  .3  ").get<FloatData>().val == Approx(0.3));
            CHECK(Object::parse("  [ ]  ").toString() == "[]");
            CHECK(Object::parse("  [  1  ,  2  ,  3  ]  ").toString() == "[1, 2, 3]");
            CHECK(Object::parse(R"(  [  1  , "hi"  ,  3  ]  )").toString() == R"([1, "hi", 3])");
            CHECK(Object::parse("  {  }  ").toString() == "{}");
            CHECK(Object::parse(R"(  {  "key"  :  14  }  )").toString() == R"({"key": 14})");
            CHECK(Object::parse(R"(  {  "key"  :  "value"  }  )").toString() == R"({"key": "value"})");

            CHECK(Object::parse("[1,2,3]").toString() == "[1, 2, 3]");
            CHECK(Object::parse(R"([1,"hi",3])").toString() == R"([1, "hi", 3])");
            CHECK(Object::parse(R"({"key":14})").toString() == R"({"key": 14})");
            CHECK(Object::parse(R"({"key":"value"})").toString() == R"({"key": "value"})");
        }
    }
}
