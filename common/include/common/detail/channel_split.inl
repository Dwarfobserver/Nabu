
class channel : public channel_base<channel> {
public:
    inline bool is_empty() const {
        return true;
    }

    template <class T>
    void send(T const& data) {

    }
    
    template <class T>
    void receive(T& data) {
        
    }
};
