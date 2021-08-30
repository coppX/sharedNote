#include <atomic>
//#include <stdatomic.h>
template<typename RefType>
class RCObject
{
public:    
    void setRef(int v)
    {
        std::atomic_store(&_ref, v);
    }

    void addRef()
    {
        std::atomic_fetch_add_explicit(&_ref, 1, std::memory_order_relaxed);
    }

    void subRef()
    {
        std::atomic_fetch_add_explicit(&_ref, -1, std::memory_order_acq_rel);
    }

    RefType getRef()
    {
        return std::atomic_load_explicit(&_ref, std::memory_order_relaxed);
    }

    bool isShared()
    {
        return getRef() > 1;
    }

private:
    std::atomic<RefType> _ref{0};
};
