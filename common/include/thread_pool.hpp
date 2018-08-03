
#pragma once

#include <reactor.hpp>


namespace nabu {

class thread_pool_type {
public:
	explicit thread_pool_type(int nb_threads);
	~thread_pool_type();

	thread_pool_type(thread_pool_type&&) = delete;

	template <class F>
	void post(F&& f);

	// The continuation will be executed by the reactor.
	template <class F, class Continuation>
	void post(F&& f, Continuation&& f2);
private:
	bool done_;
	std::vector<std::thread> threads_;
	std::queue<std::function<void()>> functors_;
	std::condition_variable cond_var_;
	std::mutex mutex_;
};

NABU_DECLARE_GLOBAL(thread_pool_type, thread_pool);


template <class F>
void thread_pool_type::post(F&& f) {
	{
		auto lock = std::lock_guard{ mutex_ };
		functors_.emplace_back([std::forward<F>(f)]{
			try { f(); }
			catch (...) {
				reactor.sync_task([exception = std::current_exception()]{
					std::rethrow_exception(exception);
				});
			}
		});
	}
	cond_var_.notify_one();
}

template <class F, class Continuation>
void thread_pool_type::post(F&& f, Continuation&& f2) {
	{
		auto lock = std::lock_guard{ mutex_ };
		functors_.emplace_back([std::forward<F>(f)] {
			try {
				if constexpr (takes_argument<Continuation>) {
					reactor.sync_task([arg = f(), f2 = std::forward<Continuation>(f2)]{
						f2(std::move(arg));
					});
				}
				else {
					f();
					reactor.sync_task(std::forward<Continuation>(f2));
				}
			}
			catch (...) {
				reactor.sync_task([exception = std::current_exception()]{
					std::rethrow_exception(exception);
				});
			}
		});
		cond_var_.notify_one();
	}
}

} // nabu
