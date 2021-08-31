# 特殊成员函数
C++类具有五种特殊成员函数:拷贝构造函数，拷贝赋值运算符，移动构造函数，移动赋值运算符，析构函数。  
其中拷贝和移动构造函数定义了当同类型的另一个对象初始化本对象时做什么  
拷贝和移动赋值运算符定义了将一个对象赋予同类型的另一个对象时做什么  
析构函数定义了此类型对象销毁时做什么  
如果没有定义这些函数，编译器自动定义缺失的操作。
可以通过= delete来阻止编译器来生成拷贝移动操作，当然析构函数不能被删除。
# 拷贝构造函数
```cpp
class demo{
public:
    demo();          //默认构造函数
    demo(const demo& d) //拷贝构造函数
    :num1(new int(*d.num1)), num2(new int(*d.num2))
    {
    }
private:
    int* num1;
    int* num2;
};
string dots(10, '.');       // 直接初始化
string s(dots);             // 拷贝初始化
string s2 = dots;           // 拷贝初始化
string null_book = "99999"; // 拷贝初始化
string nines = string(100, '9'); // 拷贝初始化
```

一般情况，拷贝构造函数是将其参数的成员逐个拷贝到正在创建的对象中。  
除了直接用=，下面情况下拷贝构造函数会被调用
- 将一个对象作为实参传递给一个非引用类型的参数
- 从一个返回类型为非引用类型的函数返回一个对象
- 用花括号列表初始化一个数组中的元素或一个聚合类中的成员

# 拷贝赋值运算符
```cpp
class demo{
public:
    demo& operator=(const demo& d) // 拷贝赋值运算符
    {
        num1 = new int(*d.num1);
        num2 = new int(*d.num2);
        return *this;
    }
};
```

拷贝构造函数和拷贝赋值运算符在调用上的区别
```cpp
string s('abc');

string s1 = s;  // 拷贝构造
string s2(s);   // 拷贝构造

string s3;
s3 = s;         // 拷贝赋值运算符
```
出现在定义语句中的就是拷贝构造函数，出现在给变量赋值语句中的就是拷贝赋值运算符

# 移动构造函数
移动操作相对于拷贝操作而已，他不会依次拷贝成员，而是窃取资源，会夺取资源的所有权，被移动的对象将不再持有资源
```cpp
class demo{
    demo(demo&& d)        //移动构造函数
    : num1(d.num1), num2(d.num2)
    {
        d.num1 = d.num2 = nullptr;
    }
};
```

# 移动赋值运算符

```cpp
class demo{
    demo& operator=(demo&& d)
    {
        num1 = d.num1;
        num2 = d.num2;
        d.num1 = d.num2 = nullptr;
        // num1 = std::move(d.num1);
        // num2 = std::move(d.num2);
    }
};
```
为什么拷贝构造等函数能访问另外一个同类型对象的私有成员?  
书上不是说私有成员不能直接访问私有成员，而是通过成员函数来访问，那上面的d.num1是怎么回事？其实这里说的不能访问私有成员是针对的类型来说的，不是针对对象来说的。这里的private是用来限制类外的东西访问类里面的东西是有限制的

# std::move


