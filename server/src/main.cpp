
#include <core.hpp>
#include <common/channel.hpp>
#include <common/messages.hpp>

int main() {

    logger.status << "server started\n";

    auto future = connect_with({ "client" });
    reactor.add_recurrent([future = std::move(future)] () mutable -> bool {
        using namespace std::literals;

        if (future.wait_for(0s) != std::future_status::ready) return false;
        
        try {
            auto c = future.get();
            logger.status << "client reached\n";
        }
        catch (connection_error const& e) {
            logger.status << "client not reached : " << e.what() << "\n";
        }
        return true;
    });
    reactor.run();
    
    logger.status << "server exited successfully\n";
    
    return 0;
}
