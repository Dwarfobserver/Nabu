
#include <catch.hpp>
#include <common/serialization.hpp>
#include <string>
#include <list>


TEST_CASE("serialization of int", "[serialization]") {
    REQUIRE(std::is_same_v<serialization_tag<int>, serial::trivial>);

    int const i = 42;
    constexpr int size = serialized_size(i);
    REQUIRE(size == sizeof(int));

    std::byte buffer[100];
    REQUIRE(serialize(i, buffer) == size);

    int i_copy = -79461;
    REQUIRE(deserialize(i_copy, buffer) == size);
    REQUIRE(i == i_copy);
}

namespace {
    struct trivial_t {
        float f;
        int i;
        char c;
    };
}

TEST_CASE("serialization of trivial struct", "[serialization]") {
    REQUIRE(std::is_same_v<serialization_tag<trivial_t>, serial::trivial>);

    trivial_t const t = { 3.14f, 36, 'c' };
    constexpr int size = serialized_size(t);
    REQUIRE(size == sizeof(trivial_t));

    std::byte buffer[100];
    REQUIRE(serialize(t, buffer) == size);

    trivial_t t_copy = {};
    REQUIRE(deserialize(t_copy, buffer) == size);

    REQUIRE(t.c == t_copy.c);
    REQUIRE(t.f == t_copy.f);
    REQUIRE(t.i == t_copy.i);
/*
    using namespace att::operators;
    bool const eq = t == t_copy;
    REQUIRE(eq);*/
}

TEST_CASE("serialization of continuous trivial (string)", "[serialization]") {
    REQUIRE(std::is_same_v<serialization_tag<std::string>, serial::continuous_trivial>);
    using namespace std::literals;

    auto const s = "Hello world !"s;
    int size = serialized_size(s);
    REQUIRE(size == sizeof(iterable_serialized_size_type) + s.size());

    std::byte buffer[100];
    REQUIRE(serialize(s, buffer) == size);

    auto s_copy = "derp"s;
    REQUIRE(deserialize(s_copy, buffer) == size);

    REQUIRE(s == s_copy);
}

TEST_CASE("serialization of iterable (list)", "[serialization]") { // TODO Map & att + paris/tuples with MSVC
    REQUIRE(std::is_same_v<serialization_tag<std::list<int>>, serial::iterable>);
    using namespace std::literals;

    auto const l = std::list<int>{ 1, 2, 3, 4 };
    int size = serialized_size(l);
    REQUIRE(size == sizeof(iterable_serialized_size_type) + l.size() * sizeof(int));

    std::byte buffer[100];
    REQUIRE(serialize(l, buffer) == size);

    auto l_copy = std::list<int>{ 5, 6 };
    REQUIRE(deserialize(l_copy, buffer) == size);

    REQUIRE(l == l_copy);
}

namespace {
    struct agg_t {
        std::string name;
        std::list<int> ints;
        float f;
    };
}

TEST_CASE("serialization of aggregate", "[serialization]") {
    REQUIRE(std::is_same_v<serialization_tag<agg_t>, serial::aggregate>);
    using namespace std::literals;

    agg_t const a = { "Aggregate"s, { 42, -18 }, 3.946f };
    int size = serialized_size(a);
    REQUIRE(size == serialized_size(a.name) + serialized_size(a.ints) + serialized_size(a.f));

    std::byte buffer[200];
    REQUIRE(serialize(a, buffer) == size);

    agg_t a_copy = { "mmmh"s, { 1 }, -7941.3f };
    REQUIRE(deserialize(a_copy, buffer) == size);
    
    REQUIRE(a.name == a_copy.name);
    REQUIRE(a.ints == a_copy.ints);
    REQUIRE(a.f    == a_copy.f);
/*
    using namespace att::operators;
    bool const eq = a == a_copy;
    REQUIRE(eq);*/
}

