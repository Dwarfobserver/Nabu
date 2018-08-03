
#include <catch.hpp>
#include <serialization.hpp>
#include <list>
#include <map>


namespace {
    template <class T, class Tag>
    void basic_test(T const& value, int expected_size, Tag) {
        using namespace nabu;
        REQUIRE(std::is_same_v<serial_tag_of<T>, Tag>);

        auto const size = serialized_size(value);
        REQUIRE(size == expected_size);

        auto buffer = std::vector<std::byte>(expected_size);
        auto stream = throw_stream{ buffer };
        stream << value;
        REQUIRE(stream.begin == buffer.data() + expected_size);
        stream.begin = buffer.data();
        REQUIRE(serialized_size(stream, type_tag<T>{}) == expected_size);

        T copy{};
        stream >> copy;
        REQUIRE(copy == value);
        REQUIRE(stream.begin == buffer.data() + expected_size);
    }
}

TEST_CASE("serialization of int", "[serialization]") {
    basic_test(42, sizeof(int), nabu::serial_tag::trivial{});
}

namespace {
    struct custom_t { int i; };

    bool operator==(custom_t lhs, custom_t rhs) { return lhs.i == rhs.i; }

    constexpr int custom_serialized_size(custom_t const&) noexcept {
        return sizeof(int) + 1;
    }
    int custom_serialized_size(nabu::buffer_span const& span, nabu::type_tag<custom_t>) noexcept {
        return sizeof(int) + 1;
    }
    void custom_serialize(nabu::buffer_span& span, custom_t const& val) {
        serialize(span, val.i);
        ++span.begin;
    }
    template <class...Ts>
    void custom_deserialize(nabu::buffer_span& span, custom_t& val) {
        deserialize(span, val.i);
        ++span.begin;
    }
}

TEST_CASE("serialization of custom type", "[serialization]") {
    basic_test(custom_t{ 42 }, sizeof(int) + 1, nabu::serial_tag::custom{});
}

TEST_CASE("serialization of std::string", "[serialization]") {
    char const messsage[] = "hello serialization !";
    int const size = sizeof(nabu::serialized_count_t) + sizeof(messsage) - 1;
    basic_test(std::string{ messsage }, size, nabu::serial_tag::array{});
}

TEST_CASE("serialization of std::list", "[serialization]") {
    int const size = sizeof(nabu::serialized_count_t) + sizeof(int) * 5;
    basic_test(std::list{ 1, 2, 3, 4, 5 }, size, nabu::serial_tag::iterable{});
}

namespace {
    struct person_t {
        int age;
        float size;
    };
    struct aggregate_t {
        int id;
        std::string name;
        std::map<std::string, person_t> relatives;
    };

    bool operator==(person_t const& lhs, person_t const& rhs) {
        return lhs.age == rhs.age && lhs.size == rhs.size;
    }

    bool operator==(aggregate_t const& lhs, aggregate_t const& rhs) {
        return lhs.id        == rhs.id   &&
               lhs.name      == rhs.name &&
               lhs.relatives == rhs.relatives;
    }
}

TEST_CASE("serialization of aggregate", "[serialization]") {
    auto const agg = aggregate_t{
        42,
        "Robert",
        { { "Patrick", { 67, 1.76f } },
          { "Leon",    { 64, 1.89f } }
        }
    };
    int const size = sizeof(int) + 
        (sizeof("Robert") + sizeof("Patrick") + sizeof("Leon")
            + 3 * (sizeof(nabu::serialized_count_t) - 1)) +
        sizeof(nabu::serialized_count_t)
            + 2 * (sizeof(int) + sizeof(float));

    basic_test(agg, size, nabu::serial_tag::aggregate{});
}

TEST_CASE("serialization failures", "[serialization]") {
    using namespace nabu;
    std::byte buffer[3];

    try {
        auto span = throw_stream{ buffer };
        span << 42;
        FAIL("throw_span didn't throw on overflow");
    }
    catch(write_buffer_overflow const&) {}

    auto span = checked_stream{ buffer };
    span << 42 << 'm';
    REQUIRE(span.error);
    REQUIRE(span.begin == buffer);

    using tested_type = std::list<char>;
    span.error = false;
    span << serialized_count_t{ 2 };

    auto const size = serialized_size(span, type_tag<tested_type>{});
    REQUIRE(size == invalid_serialized_size);
}

TEST_CASE("check serial tags", "[serialization]") {
    using namespace nabu;
    REQUIRE(std::is_same_v<serial_tag_of<int*>, serial_tag::invalid>);
}
