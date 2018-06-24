
#include <logger.hpp>
#include <iostream>

int main() {
    std::cout << "Hello server !\n";
    
    logger.status << "server exited successfully\n";
    
    return 0;
}
