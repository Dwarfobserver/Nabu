
#include <thread_pool.hpp>


namespace nabu {

thread_pool_type::thread_pool_type(int const nb_threads) :
	done_{ false }
{
	NABU_ASSERT(nb_threads > 0, "The thread pool must have at least one worker");

	threads_.reserve(nb_threads);
	for (auto i = 0; i < nb_thread; ++i) {
		threads_.emplace_back([this] {
			auto f = fu2::unique_function<void()>{};
			while (true) {
				{
					auto lock = std::unique_lock{ mutex_ };
					cond_var_.wait(lock, [this] {
						return done_ || !functors_.empty();
					});
					if (done_ && functors_.empty()) break;

					f = std::move(functors_.front());
					functors_.pop();
				}
				f();
			}
		});
	}
}

thread_pool_type::~thread_pool_type() {
	{
		auto lock = std::lock_guard{ mutex_ };
		done_ = true;
	}
	cond_var_.notify_all();
	for (auto& thread : threads_) {
		thread.join();
	}
	threads_.clear();
}

NABU_IMPLEMENT_GLOBAL(thread_pool, std::thread::hardware_concurrency() - 1);

} // nabu

