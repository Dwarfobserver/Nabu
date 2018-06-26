
#pragma once

#include <string>

#if   defined(MERGE_CLIENT)
    #define main(...) _client_main(__VA_ARGS__)
#elif defined(MERGE_SERVER)
    #define main(...) _server_main(__VA_ARGS__)
#endif

struct person_id {
    std::string id;
};

inline person_id server_id = { "server" };
