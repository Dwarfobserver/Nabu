
#include <allocators.hpp>


namespace nabu {

NABU_IMPLEMENT_GLOBAL(global_allocator,
    [NABU_CAPTURE_DESTRUCTOR(logger.debug("Global allocator created"))] {
        using resource_t = global_allocator_resource_type;
        using alloc_t = block_allocator<global_allocator_value_type, global_allocator_resource_type>;

        static resource_t resource{ 2048, 1.5f };
        return alloc_t{ resource };
    } ());

} // nabu
