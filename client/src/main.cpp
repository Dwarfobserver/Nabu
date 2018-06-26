
#include <core.hpp>
#include <common/channel.hpp>
#include <common/messages.hpp>
#include <glfw3.h>

int main() {
    /*
    if (!glfwInit()) return -1;

    auto window = glfwCreateWindow(640, 480, "Client", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwTerminate();*/

    logger.status << "client started\n";

    auto future = connect_with(server_id);
    reactor.add_recurrent([future = std::move(future)] () mutable -> bool {
        using namespace std::literals;

        if (future.wait_for(0s) != std::future_status::ready) return false;
        
        try {
            auto c = future.get();
            logger.status << "server reached\n";
        }
        catch (connection_error const& e) {
            logger.status << "server not reached : " << e.what() << "\n";
        }
        return true;
    });
    reactor.run();

    logger.status << "client exited successfully\n";
    
    return 0;
}
