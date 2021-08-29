#include <memory>

template<typename T>
class Allocator
{
public:
    typedef T                               value_type;
    typedef T*                              pointer;
    typedef const T*                        const_pointer;  
    typedef T&                              reference;
    typedef const T&                        const_reference;
    typedef std::size_t                     size_type;
public:
    pointer address(reference x) const
    {
        return std::addressof(x);
    }

    const_pointer address(const_reference x) const
    {
        return std::addressof(x);
    }

    //member
    pointer allocate(size_type n)
    {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(value_type))
        {
            throw std::bad_alloc();
        }
        if (pointer p = static_cast<pointer>(std::malloc(n * sizeof(value_type))))
        {
            return p;
        }
        throw std::bad_alloc();
    }

    void deallocate(pointer p, size_type n = 0)
    {
        if (nullptr != p)
        {
            std::free(p);
        }
    }

    void construct(pointer p)
    {
        ::new((void*)p) value_type();
    }

    template<typename U, typename... Args>
    void construct(U* up, Args&&... args)
    {
        ::new((void*)up) U(std::forward<Args>(args)...);
    }

    template<typename pointer>
    typename std::enable_if<!std::is_trivially_destructible<pointer>::value>::type
    destroy(pointer p)
    {
        p->~value_type();
    }

    template<typename pointer>
    typename std::enable_if<std::is_trivially_destructible<pointer>::value>::type    
    destroy(pointer p)
    {

    }
};

// template<typename Alloc>
// class Allocator_Traits
// {
// private:
//     typedef Alloc                                       allocator_type;
//     typedef typename allocator_type::value_type         value_type;

// };
