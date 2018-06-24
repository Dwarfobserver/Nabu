
#if   defined(MERGE_CLIENT)
    #define main(...) _client_main(__VA_ARGS__)
#elif defined(MERGE_SERVER)
    #define main(...) _server_main(__VA_ARGS__)
#endif
