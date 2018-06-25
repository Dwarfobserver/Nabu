
#include <future>

int _client_main();
int _server_main();

int main() {
    auto client = std::async(std::launch::async, _client_main);
    int server_res = _server_main();
    int client_res = client.get();

    if (server_res != 0 || client_res != 0) return 1;
}
