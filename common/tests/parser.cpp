
#include <catch.hpp>
#include <msg/parser.hpp>


namespace {
    struct message_t {
        static constexpr nabu::id_type id = 12;

        int num;
        std::string name;
    };
    
    bool operator==(message_t const& lhs, message_t const& rhs) {
        return lhs.num == rhs.num && lhs.name == rhs.name;
    }
}

TEST_CASE("test message", "[parser]") {
    std::byte buffer[100];
    auto span = nabu::throw_stream{ buffer };

    auto const msg = message_t{ 42, "hello" };

    auto sender = nabu::msg::parser{};
    sender.serialize(span, msg);

    auto received = nabu::msg::parser{};
    bool triggered = false;
    received.set_callback([&] (message_t&& m) {
        triggered = true;
        REQUIRE(m == msg);
    });

    span.begin = buffer;
    received.deserialize(span);
    REQUIRE(triggered);
}
