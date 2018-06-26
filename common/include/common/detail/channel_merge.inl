
#include <vector>
#include <mutex>
#include <map>

namespace detail {

    class ring_buffer {
    public:
        ring_buffer(int capacity) :
                vec_(capacity),
                consumer_(0),
                producer_(0)
        {
            if (capacity & (capacity - 1) != 0) throw std::logic_error
                {"ring_buffer capacity must be a multiple of two"};
        }

        template <class T>
        void send(T const& data) {
            auto l = std::lock_guard{ mutex_ };

            // TODO
        }
        
        template <class T>
        void receive(T& data) {
            auto l = std::lock_guard{ mutex_ };
            
            // TODO
        }

        bool is_empty() const {
            auto l = std::lock_guard{ mutex_ };

            return consumer_ == producer_;
        }
    private:
        std::vector<char> vec_;
        mutable std::mutex mutex_;
        int consumer_;
        int producer_;
    };

    inline ring_buffer client_buffer(4'096);
    inline ring_buffer server_buffer(4'096);
}

class channel : public channel_base<channel> {
public:
    inline bool is_empty() const {
        return true;
    }

    template <class T>
    void send(T const& data) {
        // TODO
    }
    
    template <class T>
    void receive(T& data) {
        // TODO
    }

};
