
#pragma once

#include <serialization.hpp>
#include <reflection.hpp>
#include <boost/callable_traits/args.hpp>
#include <function2.hpp>
#include <vector>


namespace nabu::msg {

struct parser_tokens {
    char begin;
    char separator;
    char end;
};

class parser_error : public std::runtime_error {
public:
    inline explicit parser_error(std::string const& str) :
        std::runtime_error{ str } {}

    template <class T1, class T2>
    parser_error(std::string const& symbol, T1 const& expected, T2 const& got):
        std::runtime_error{ fmt::format(
            "Invalid {} : expected '{}' but got '{}' instead",
            symbol, expected, got
        )} {}
};

class parser {
    using callback_t = fu2::unique_function<void(buffer_span&)>;

    parser_tokens tokens_;
    std::vector<callback_t> callbacks_;
public:
    inline explicit parser(parser_tokens tokens = { '{', ':', '}' }) :
        tokens_{ tokens }, callbacks_(id_max_value + 1) {}
    
    template <class Message, class Policy>
    void serialize(basic_stream<Policy>& stream, Message const& msg) {
        static_assert(has_id<Message> && is_serializable<Message>);

        stream << tokens_.begin
               << id_of<Message>()
               << tokens_.separator
               << msg
               << tokens_.end;
    }

    template <class Policy>
    void deserialize(basic_stream<Policy>& stream) {
        char token;
        stream >> token;
        if (token != tokens_.begin) throw parser_error
            { "message begin token", tokens_.begin, token };
                
        id_type id;
        stream >> id;
        if (!is_valid_id(id)) throw parser_error
            { "message id", fmt::format("value in [{}, {}]", id_min_value, id_max_value), id };
        
        auto& f = callbacks_[id];
        if (!f) throw parser_error { fmt::format(
            "invalid message id : no callback for id '{}'", id) };

        stream >> token;
        if (token != tokens_.separator) throw parser_error
            { "message separator token", tokens_.separator, token };
        
        f(stream);
    }

    template <class F>
    void set_callback(F&& f) {
        using Message = std::remove_reference_t<first_arg_of<F>>;
        static_assert(has_id<Message> && is_serializable<Message>,
            "The callback [F] argument must validates 'had_id' and 'is_serializable'");
        
        auto const id = id_of<Message>();

        callbacks_[id] = [this, f = std::forward<F>(f)] (buffer_span& span) {
            Message msg;
            throw_deserialize(span, msg);
            char token;
            throw_deserialize(span, token);
            if (token != tokens_.end) throw parser_error
                { "message separator token", tokens_.end, token };
            
            f(std::move(msg));
        };
    }
};

} // nabu::msg
