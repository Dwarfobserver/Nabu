
#include <logger.hpp>
#include <glfw3.h>
#include <iostream>

int main() {
    if (!glfwInit()) return -1;

    auto window = glfwCreateWindow(640, 480, "Client", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwTerminate();

    logger.status << "client exited successfully\n";
    
    return 0;
}
