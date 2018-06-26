
#include <vector>
#include <mutex>

detail::ring_buffer::ring_buffer(int capacity) :
        vec_(capacity),
        consumer_(0),
        producer_(0)
{}

void detail::ring_buffer::send(void const* data, int size) {
    auto l = std::lock_guard{ mutex_ };

    auto ptr = static_cast<char const*>(data);
    vec_.insert(vec_.end(), ptr, ptr + size);
}

int detail::ring_buffer::receive(void* data, int max_size) {
    auto l = std::lock_guard{ mutex_ };

    int size = vec_.size();
    if (max_size < size) throw std::runtime_error{"Channel buffer overflow"};

    std::memcpy(data, vec_.data(), size);
    vec_.clear();
    return size;
}


channel make_channel(person_id const& src, person_id const& dst) {
    return {};
}
