
#include <reactor.hpp>


namespace nabu {

NABU_IMPLEMENT_GLOBAL(reactor, 
    [NABU_CAPTURE_DESTRUCTOR(logger.debug("Reactor created"))] {
        return global_allocator;
    } ());

} // nabu
