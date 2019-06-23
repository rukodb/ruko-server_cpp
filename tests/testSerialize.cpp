#include "libs/catch.hpp"
#include "../source/RukoServer.hpp"

template<typename T>
T deserialize(const Bytes &bytes) {
    const Byte *data = bytes.data();
    size_t p = 0;
    return deserialize<T>(data, p);
}

template<typename T>
void checkSerialize(T t) {
    CHECK(deserialize<T>(serialize(t)) == t);
}

template<typename T, typename...Args>
void checkSerialize(T t, Args... args) {
    CHECK(deserialize<T>(serialize(t)) == t);
    checkSerialize(args...);
}


TEST_CASE("Serialization") {
    SECTION("primitives") {
        SECTION("unsigned int") {
            Bytes bytes = u32ToBytes(113);
            size_t p = 0;
            auto len = bytesToU32(bytes.data(), p);
            CHECK(len == 113);
            CHECK(p == 4);
        }
        SECTION("signed int") {
            Bytes bytes = i32ToBytes(-113);
            size_t p = 0;
            auto len = bytesToI32(bytes.data(), p);
            CHECK(len == -113);
            CHECK(p == 4);
        }
        SECTION("many") {
            checkSerialize(
                    std::string("hello"), std::string(""),
                    12.2f, 0.f,
                    0, 1, 5, 512,
                    true, false,
                    std::vector<std::string>({}),
                    std::vector<std::string>({"hi", "hello", ""}),
                    Object::parse(R"(null)"),
                    Object::parse(R"(12)"),
                    Object::parse(R"(10)"),
                    Object::parse(R"(12.5)"),
                    Object::parse(R"(0.0)"),
                    Object::parse(R"("this is a string")"),
                    Object::parse(R"("")"),
                    Object::parse(R"([])"),
                    Object::parse(R"(["this", "is", "an", "array"])"),
                    Object::parse(R"({})"),
                    Object::parse(R"({"hello": "hi"})")
            );
        }
    }
}
