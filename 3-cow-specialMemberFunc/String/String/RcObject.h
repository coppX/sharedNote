template<typename RefType>
class RCObject
{
public:
    RCObject()
    : _ref(0)
    , _shareable(true)
    {}
    RCObject(const RCObject& obj)
    {
        _ref = obj._ref;
        _shareable = obj._shareable;
    }

    ~RCObject()
    {}
    
    void setRef(RefType v)
    {
        _ref = v;
    }

    RefType addRef()
    {
        return ++_ref;
    }

    RefType subRef()
    {
        return --_ref;
    }

    RefType getRef()
    {
        return _ref;
    }

private:
    RefType _ref;
    bool _shareable;
};