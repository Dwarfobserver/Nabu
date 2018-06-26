
#include <common/reactor_base.hpp>
#include <chrono>
#include <thread>


void reactor_base::add_oneshot(fu2::unique_function<void()>&& f) {
    oneshots_.push_back(std::move(f));
}

void reactor_base::add_recurrent(fu2::unique_function<bool()>&& f) {
    recurrents_.push_back(std::move(f));
}

void reactor_base::run() {
    using namespace std::chrono;
    using namespace std::literals;

    while (!oneshots_.empty() || !recurrents_.empty())
    {
        auto tbegin = high_resolution_clock::now();

        int size = oneshots_.size();
        for (int i = 0; i < size; ++i) {
            oneshots_.front()();
            oneshots_.pop_front();
        }
        size = recurrents_.size();
        for (int i = 0; i < size; ++i) {
            bool finished = recurrents_.front()();
            if (finished) recurrents_.pop_front();
        }
        
        auto tend = high_resolution_clock::now();

        std::this_thread::sleep_for(20ms - (tend - tbegin));
    }
}

