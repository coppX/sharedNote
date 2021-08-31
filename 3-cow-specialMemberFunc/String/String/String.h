#include <atomic>
#include <cassert>
#include "Allocator.h"
#include "RcObject.h"

// C++11之后规定string不允许使用COW(Copy on Write)技术实现，改用SSO(Small Basic_String Optimization)
// https://stackoverflow.com/questions/12199710/legality-of-cow-stdstring-implementation-in-c11
// 并且多线程中COW实现的string会变慢

// SSO将基于大多数字符串比较短的特点，利用string对象本身的栈空间来存放短字符串。当字符串长度大于某个值时，使用eager copy的方式。
// SSO将字符串分为短字符串和长字符串两种场景

// 这里为了学习COW，本例子采用COW
template<typename CharT, typename AllocT = Allocator<CharT> >
class Basic_String 
{
private:
    typedef CharT                                   value_type;
    typedef std::size_t                             size_type;
    typedef AllocT                                  allocator_type;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef typename allocator_type::pointer        pointer;
    typedef pointer                                 iterator;
    typedef const iterator                          const_iterator;
    typedef RCObject<int>                           refcount_type;
public:
    //constructor
    Basic_String()
    {}

    ~Basic_String()
    {
        if (_value._data)
        {
            _value._refCount->subRef();
            if (_value._refCount->getRef() <= 0)
            {
                get_allocator().deallocate(_value._data);
                delete _value._refCount;
            }
            _value._data = nullptr;
            _value._size = 0;
            _value._capacity = 0;
            _value._refCount = nullptr;
        }
    }

    Basic_String(const CharT* s)
    {
        size_type len = strlen(s) / sizeof(value_type);
        _init(s, len);
        _setSize(len);
    }

    Basic_String(const CharT* s, size_type n)
    {
        _init(s, n);
        _setSize(n);
    }

    Basic_String(size_type n, CharT c)
    {
        
    }

    Basic_String(const Basic_String& Other)
    {
        _value = Other._value;
        Other._value._refCount->addRef();
        get_allocator() = Other.get_allocator();
    }

    Basic_String(Basic_String&& Other)
        : _value(std::move(Other._value))
    {
        get_allocator() = std::move(Other.get_allocator());
    }

    //Basic_String(Basic_String&& Other, const allocator_type& Alloc);

    Basic_String& operator=(Basic_String& Other)
    {
        if (_value._data != Other._value._data) 
        {
            //浅拷贝内存，原来字符串的的计数减一
            if (_value._data)
            {
                _value._refCount->subRef();
                if (_value._refCount->getRef() <= 0)
                {//释放掉原来的空间
                    get_allocator().deallocate(_value._data);
                    delete _value._refCount;
                    _value._refCount = nullptr;
                }
            }
            _value = Other._value;
            //新的引用计数+1
            Other._value._refCount->addRef();
            
            get_allocator() = Other.get_allocator();
        }
        return *this;
    }

    Basic_String operator=(Basic_String&& Other)
    {
        _value = std::move(Other._value);
        get_allocator() = std::move(Other.get_allocator());
        return *this;
    }
    
    // -------------------------
    // element access
    // -------------------------
    reference at(size_type pos)
    {
        assert(pos < size() && "pos out of range");
        return (*this)[pos];
    }

    // 如果用COW实现，那么 non-const operator[] 可能会导致迭代器失效。
    // 而标准严格规定了哪些成员方法可以导致迭代器失效，其中不包括这个方法
    // per the standard 21.4.1 p6, invalidation of iterators/references is only allowed for
    // 根据标准21.4.1 p6,仅允许迭代器/引用失效:
    // — as an argument to any standard library function taking a reference to non-const basic_string as an argument.
    // 当non-const basic_string作为任何标准库函数的参数
    // — Calling non-const member functions, except operator[], at, front, back, begin, rbegin, end, and rend.
    // 调用非成员函数，除了operator[], at, front, back, begin, rbegin, end和rend除外


    // 也就是operator[]函数不允许返回的迭代器/引用失效，但是COW实现的string在operator[]操作时会进行内存拷贝操作，这会使引用失效。
    // 例子来源: https://stackoverflow.com/a/12199969
    // std::string a("something"); c
    // har& c1 = a[0]; 
    // std::string b(a);  因为这里有两个string指向同一个内存buffer
    // char& c2 = a[1];   所以这里的operator[]操作会引起COW
    // 这里的COW操作会使上面的c1引用失效，虽然此时c1还保留值's'，但是此时这块内存只是string b在使用，c1就没有指向a了，因为a已经COW了
    reference operator[] (size_type pos)
    {
        assert(pos < size() && "string out of index");
        //copy on write!
        if (_value._refCount->isShared()) 
        {
            _value._refCount->subRef();
            _realloc();
        }
        return *(data() + pos);
    }

    reference front()
    {
        assert(!empty() && "string is empty");
        return *data();
    }

    reference back()
    {
        assert(!empty() && "string is empty");
        return *(data() + size() - 1);
    }

    value_type* data() const
    {
        return _value._data;
    }
    
    value_type* c_str() const
    {
        return data();
    }

    // -------------------
    // iterators
    // -------------------
    iterator begin()
    {
        return data();
    }

    const_iterator cbegin() const
    {
        return data();
    }

    iterator end()
    {
        return data() + size() - 1;
    }

    const_iterator cend() const
    {
        return data() + size() - 1;
    }

    // -------------------
    //capacity
    // -------------------
    bool empty() const
    {
        return size() == 0;
    }

    size_type size() const
    {
        return _value._size;
    }
    size_type length() const
    {
        return size();
    }

    size_type max_size()
    {

    }

    void reserve(size_type new_cap = 0)
    {

    }

    size_type capacity() const
    {
        return _value._capacity;
    }

    void shrink_to_fit()
    {

    }

    allocator_type get_allocator() const
    {
        return _allocator;
    }

    // ----------------------
    // operatotions
    // ----------------------
    void clear()
    {
        _setSize(0);
    }

    iterator insert(iterator pos, value_type ch)
    {
        
    }

    iterator erase(iterator pos)
    {

    }

    void push_back(value_type ch)
    {

    }

    void pop_back()
    {

    }

    int compare(const Basic_String& str) const
    {

    }

    Basic_String substr(size_type pos = 0, size_type count = 0) const
    {

    }

    size_type copy(value_type* dest, size_type count, size_type pos = 0) const
    {

    }

    void resize(size_type count)
    {

    }

    void swap(Basic_String& other)
    {

    }

private:
    struct value
    {
        value_type*          _data;
        size_type            _size;
        size_type            _capacity;
        refcount_type*       _refCount;

        value()
        : _data(nullptr)
        , _size(0)
        , _capacity(0)
        , _refCount(new refcount_type())
        {}

        value(const value& other)
        {
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            _refCount = other._refCount;
        }

        const value& operator=(const value& other)
        {
            _data = other._data;
            _size = other._size;
            _capacity = other._capacity;
            _refCount = other._refCount;
            return *this;
        }
    }                       _value;
    
    allocator_type          _allocator;

private:
    void _init(const CharT* s, size_type n)
    {
        _value._capacity = 2 * n;
        _value._data = _allocator.allocate(_value._capacity);
        std::memcpy(_value._data, s, n);
        _value._data[n] = '\0';
        _value._refCount->setRef(1);
    }

    void _realloc()
    {
        value_type* temp = _allocator.allocate(_value._capacity);
        std::memcpy(temp, _value._data, _value._size);
        _value._data = temp;
        _value._data[_value._size] = '\0';
        _value._refCount = new refcount_type();
        _value._refCount->setRef(1);
    }

    void _setSize(size_type s)
    {
        _value._size = s;
    }
};

using String = Basic_String<char>;
