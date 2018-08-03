
#pragma once

#include <logger.hpp>
#include <allocators.hpp>
#include <reflection.hpp>
#include <boost/callable_traits/args.hpp>
#include <function2.hpp>
#include <vector>
#include <chrono>
#include <mutex>
#include <queue>
#include <list>


namespace nabu {

// TODO Add delayed callback ?
// TODO Implement template methods outside
// TODO Event SBO customisable ('smart pointer' allowed ?)
template <class EventAllocator>
class basic_reactor {

    // TODO callback block allocator
    using callback_t = fu2::unique_function<bool(void*)>;

    using timepoint_t = decltype(std::chrono::steady_clock::now());

    template <class Event>
    using allocator_traits_t = typename std::allocator_traits<EventAllocator>::template rebind_traits<Event>;

    template <class Event>
    using allocator_t = typename std::allocator_traits<EventAllocator>::template rebind_alloc<Event>;

    class callback_list {
        std::list<callback_t> callbacks_; // TODO block allocator
    public:
        using destructor_t = void (*) (EventAllocator&, void*);
        destructor_t destructor;

        callback_list() noexcept : destructor{ nullptr } {}

        template <class F>
        void add(F&& f) {
            callbacks_.emplace_back(std::forward<F>(f));
        }

        void consume(void* const event, EventAllocator& alloc) {
            auto const end = callbacks_.end();
            auto it = this->callbacks_.begin();
            while (it != end) {
                bool finished = true;
                try {
                    finished = (*it)(event);
                }
                catch (std::runtime_error const& e) {
                    logger.error(fmt("Error '{}' caught in reactor :\n{}"), name_of(e), e.what());
                }
                if (finished) {
                    it = callbacks_.erase(it);
                }
                else {
                    ++it;
                }
            }
            destructor(alloc, event);
        }
    };

    struct tagged_event {
        id_type id;
        void* event;
    };
    struct timed_event {
        timepoint_t timeout;
        id_type id;
        void* event;
    };

    static constexpr auto timed_event_comparer =
        [] (timed_event const& lhs, timed_event const& rhs) noexcept {
            return lhs.timeout < rhs.timeout;
        };
    
    using priority_queue_t = std::priority_queue<
        timed_event,
        std::vector<timed_event>,
        decltype(timed_event_comparer)
    >;

    template <class Event>
    static Event* create_event(EventAllocator& alloc, Event&& event) {
        using T = std::remove_reference_t<Event>;
        using allocator = allocator_t<T>;
        using traits = allocator_traits_t<T>;
        
        auto a = allocator{ alloc };
        auto const ptr = traits::allocate(a, 1);
        traits::construct(a, ptr, std::forward<Event>(event));
        return ptr;
    }
    template <class Event>
    static void destroy_event(EventAllocator& alloc, void* const ptr) {
        using allocator = allocator_t<Event>;
        using traits = allocator_traits_t<Event>;

        auto a = allocator{ alloc };
        auto const event = static_cast<Event*>(ptr);
        traits::destroy(a, event);
        traits::deallocate(a, event, 1);
    }

    template <class Event>
    using id_expression = decltype(Event::id);

    template <class Event>
    static constexpr void check_class() {
        static_assert(is_detected<id_expression, Event>,
            "[Event] class must have a public static positive integral value named 'id'");
        static_assert(Event::id >= 0,
            "[Event] class must have a public static positive integral value named 'id'");
    }

    EventAllocator allocator_;
    std::vector<callback_list> callbacks_;
    std::mutex tasks_mutex_;
    std::vector<fu2::unique_function<void()>> sync_tasks_;
	std::list<fu2::unique_function<bool()>> routines_;
    std::list<tagged_event> events_;
    priority_queue_t delayed_events_;
public:
    basic_reactor(EventAllocator const& allocator = EventAllocator{}) noexcept :
        allocator_{ allocator },
        delayed_events_{ timed_event_comparer }
    {
        logger.debug("Reactor created");
    }

    template <class F>
    void sync_task(F&& f) {
        std::lock_guard<std::mutex> g{ tasks_mutex_ };
        sync_tasks_.emplace_back(std::forward<F>(f));
    }

	template <class F>
	void add_routine(F&& f)
    {
		routines_.emplace_back(std::forward<F>(f));
    }

    template <class Event>
    void register_event() {
        check_class<Event>();
        constexpr auto id = Event::id;

        if (callbacks_.size() < id + 1) {
            callbacks_.resize(id + 1);
        }
        if (callbacks_[id].destructor == nullptr) {
            callbacks_[id].destructor = destroy_event<Event>;
        }
    }

    template <class Event>
    bool is_registered() const noexcept {
        check_class<Event>();
        constexpr auto id = Event::id;

        return callbacks_.size() >= id + 1 && callbacks_[id].destructor != nullptr;
    }

    template <class F>
    void subscribe(F&& f) {
        using Args = boost::callable_traits::args_t<F>;
        using Event = std::remove_reference_t<decltype(std::get<0>(std::declval<Args>()))>;

        // TODO Register through static function value
        NABU_ASSERT(is_registered<Event>(), name_of<Event>() + " must be registered before calling"
               "'add(f: " + name_of<Event>() + " const& -> bool)'");

        constexpr auto id = Event::id;

        callbacks_[id].add([f = std::forward<F>(f)] (void* ptr) -> bool {
            auto event = static_cast<Event const*>(ptr);
            return f(*event);
        });
    }

    template <class Event>
    void notify(Event&& event) {
        using EventT = std::remove_reference_t<Event>;

        NABU_ASSERT(is_registered<EventT>(), name_of<EventT>() + " must be registered before calling"
               "'notify(" + name_of<Event>() + ")'");

        constexpr auto id = EventT::id;
        
        auto const ptr = create_event(allocator_, std::forward<Event>(event));
        events_.push_back({ id, ptr });
    }
    
    template <class Event, class Rep, class Period>
    void notify(Event&& event, std::chrono::duration<Rep, Period> const cd) {
        using EventT = std::remove_reference_t<Event>;

        NABU_ASSERT(is_registered<EventT>(), name_of<EventT>() + " must be registered before calling"
               "'notify(" + name_of<Event>() + ", " + name_of<decltype(cd)>() + ")'");

        constexpr auto id = EventT::id;

        auto const ptr = create_event(allocator_, std::forward<Event>(event));
        auto const time = std::chrono::steady_clock::now() + cd;
        delayed_events_.push({ time, id, ptr });
    }

    bool is_empty() const noexcept {
        return events_.empty() && delayed_events_.empty();
    }

    void update() {
		// synchronized tasks
        decltype(sync_tasks_) tasks;
        {
            std::lock_guard<std::mutex> g{ tasks_mutex_ };
            tasks = std::move(sync_tasks_);
        }
        for (auto& task : tasks) task();

		// routines
		auto it = routines_.begin();
		while (it != routines_.end())
		{
			if ((*it)()) routines_.erase(it);
			else ++it;
		}

		// events
        int const event_count = events_.size();
        for (int i = 0; i < event_count; ++i) {
            auto const node = events_.front();
            events_.pop_front();
            auto& cb = callbacks_[node.id];
            cb.consume(node.event, allocator_);
        }

		// delayed events
        auto const now = std::chrono::steady_clock::now();
        while (!delayed_events_.empty()) {
            auto const node = delayed_events_.top();
            if (node.timeout > now) break;
            delayed_events_.pop();
            auto& cb = callbacks_[node.id];
            cb.consume(node.event, allocator_);
        }
    }
};

NABU_DECLARE_GLOBAL(basic_reactor<global_allocator_type<>>, reactor);

} // nabu
