
#pragma once

#include <logger.hpp>
#include <meta.hpp>
#include <memory>
#include <vector>
#include <string>


namespace nabu {

// The block allocator allocate chunks of memory of the same size.

template <
    class T,
    class Resource
>
class block_allocator {
    Resource& resource_;
public:
    using value_type = T;
    using resource_type = Resource;

    static constexpr size_t max_continuous_allocation = Resource::block_size / sizeof(T);

    Resource& get_resource() const noexcept { return resource_; } 

    explicit block_allocator(Resource& resource) noexcept :
        resource_{ resource }
    {}

    template <class U>
    block_allocator(block_allocator<U, Resource> const& allocator) noexcept :
        block_allocator{ allocator.get_resource() }
    {}

    [[nodiscard]]
    T* allocate(size_t const n) {
        NABU_ASSERT(n <= max_continuous_allocation,
            "{} objects of type '{}' tried to be allocated continuously, but only {} can fit in the same resource block",
            n, name_of<T>(), max_continuous_allocation);
        
        return static_cast<T*>(resource_.allocate());
    }
    
    void deallocate(T* const ptr, size_t const n) noexcept {
        resource_.deallocate(ptr);
    }
};

// This resource allocate chunks of memory per growing groups.

template <
    size_t BlockSize
    // TODO class Allocator = std::allocator<char>
>
class block_allocator_resource {
    struct alignas(8) block_t {
        union {
            block_t* next;
            char storage[BlockSize];
        };
    };
    int next_chunk_size_;
    float grow_factor_;
    std::vector<std::vector<block_t>> chunks_;
    block_t* head_;

    void add_chunk() {
        auto& chunk = chunks_.emplace_back(next_chunk_size_);

        auto const last_index = next_chunk_size_ - 1;
        next_chunk_size_ = static_cast<int>(static_cast<float>(next_chunk_size_) * grow_factor_);

        for (int i = 0; i < last_index; ++i) {
            chunk[i].next = &chunk[i + 1];
        }
        chunk[last_index].next = nullptr;
        head_ = &chunk.front();
    }
public:
    static constexpr size_t block_size = BlockSize;

    explicit block_allocator_resource(int const chunk_size, float const grow_factor = 2.f) :
        next_chunk_size_{ chunk_size },
        grow_factor_    { grow_factor }
    {
        NABU_ASSERT(grow_factor >= 1.f, "block_allocator_resource 'grow_factor' must be >= 1.f");
        add_chunk();
    }

    [[nodiscard]]
    void* allocate() {
        if (head_ == nullptr) add_chunk();
        
        auto const old_head = head_;
        head_ = head_->next;
        return old_head;
    }

    void deallocate(void* ptr) {
        auto const block = static_cast<block_t*>(ptr);
        block->next = head_;
        head_ = block;
    }
};

template <
    class T,
    class FirstAllocator,
    class BackupAllocator,
    template <class> class ChooseFirstPredicate
>
class hybrid_allocator {
    template <class, class, class, template <class> class>
    friend class hybrid_allocator;

    using predicate_type = ChooseFirstPredicate<T>;

    FirstAllocator  first_allocator_;
    BackupAllocator backup_allocator_;

    template <class U> using rebind_first_type  =
        typename std::allocator_traits<FirstAllocator >::template rebind_alloc<U>;

    template <class U> using rebind_second_type =
        typename std::allocator_traits<BackupAllocator>::template rebind_alloc<U>;

    template <class U> using rebind_allocator = 
        hybrid_allocator<
            U,
            rebind_first_type<U>,
            rebind_second_type<U>,
            ChooseFirstPredicate
        >;
public:
    using value_type = T;

    template <class U>
    struct rebind {
        using other = rebind_allocator<U>;
    };

    hybrid_allocator(FirstAllocator  const& first_allocator  = FirstAllocator{},
                     BackupAllocator const& backup_allocator = BackupAllocator{}) :
        first_allocator_ { first_allocator },
        backup_allocator_{ backup_allocator }
    {
        static_assert(std::is_same_v<T, FirstAllocator::value_type> && 
                      std::is_same_v<T, BackupAllocator::value_type>);
    }

    template <class U>
    hybrid_allocator(rebind_allocator<U> const& allocator) :
        first_allocator_ { allocator.first_allocator_ },
        backup_allocator_{ allocator.backup_allocator_ }
    {
        static_assert(std::is_same_v<T, FirstAllocator::value_type> && 
                      std::is_same_v<T, BackupAllocator::value_type>);
    }

    [[nodiscard]]
    T* allocate(size_t const n) {
        return predicate_type{}(n) 
            ? first_allocator_. allocate(n)
            : backup_allocator_.allocate(n);
    }
    
    void deallocate(T* const ptr, size_t const n) noexcept {
        if (predicate_type{}(n))
             first_allocator_. deallocate(ptr, n);
        else backup_allocator_.deallocate(ptr, n);
    }
};

template <class T, class Resource>
struct hybrid_block_allocator_predicate {
    constexpr bool operator()(size_t const n) const noexcept {
        auto constexpr max_n = block_allocator<T, Resource>::max_continuous_allocation;
        if constexpr (max_n == 0)
             return false;
        else return n <= max_n;
    }
};

template <class T, class Resource, class BackupAllocator>
using hybrid_block_allocator = hybrid_allocator<
    T,
    block_allocator<T, Resource>,
    BackupAllocator,
    curry_right<hybrid_block_allocator_predicate, Resource>::type
>;

using global_allocator_resource_type = block_allocator_resource<4>;

using global_allocator_value_type = int;

template <class T = global_allocator_value_type>
using global_allocator_type = 
    hybrid_block_allocator<
        T,
        global_allocator_resource_type,
        std::allocator<T>
    >;

NABU_DECLARE_GLOBAL(global_allocator_type<>, global_allocator);

} //  nabu
