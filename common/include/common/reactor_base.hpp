
#pragma once

#include <function2.hpp>
#include <list>

class reactor_base {
public:
    void add_oneshot(fu2::unique_function<void()>&& f);
    void add_recurrent(fu2::unique_function<bool()>&& f);

    void run();
private:
    std::list<fu2::unique_function<void()>> oneshots_;
    std::list<fu2::unique_function<bool()>> recurrents_;
};
