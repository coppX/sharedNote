# 特殊成员函数篇
C++类具有五种特殊成员函数:拷贝构造函数，拷贝赋值运算符，移动构造函数，移动赋值运算符，析构函数。  
其中拷贝和移动构造函数定义了当同类型的另一个对象初始化本对象时做什么  
拷贝和移动赋值运算符定义了将一个对象赋予同类型的另一个对象时做什么  
析构函数定义了此类型对象销毁时做什么  
如果没有定义这些函数，编译器自动定义缺失的操作。
可以通过`= delete`来阻止编译器来生成拷贝移动操作，当然析构函数不能被删除。
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
    demo& operator=(demo&& d)   //这里的参数不能是const
    {
        num1 = d.num1;
        num2 = d.num2;
        d.num1 = d.num2 = nullptr;
        // num1 = std::move(d.num1);
        // num2 = std::move(d.num2);
        return *this;
    }
};
```
为什么拷贝构造等函数能访问另外一个同类型对象的私有成员?  
书上不是说私有成员不能直接访问私有成员，而是通过成员函数来访问，那上面的`d.num1`是怎么回事？其实这里说的不能访问私有成员是针对的类型来说的，不是针对对象来说的。这里的`private`是用来限制类外的东西访问类里面的东西是有限制的

## 析构函数
和构造函数的操作相反， 构造函数初始化对象的非static数据成员，析构函数是否对象使用的资源，并销毁对象的非static数据成员。析构函数没有参数和返回值，所以他不能被重载，一个类只能有一个析构函数。  
在一个构造函数中，成员的初始化是在函数体执行之前完成的，而且按照它们在类中出现的顺序进行初始化。在一个析构函数中，首先执行函数体，然后再销毁成员，成员按照初始化顺序逆序销毁。

# 移动语义篇
## 引用折叠和右值引用参数
### 左值 & 右值
当一个对象被用作右值的时候，用的是对象的值(内容),当对象被用作左值的时候，用的是对象的身份(在内存中的位置)  
返回左值引用的函数，连同赋值、下标、解引用和前置递增/递减运算符，都是返回左值的表达式的例子。我们可以将一个左值引用绑定到这类表达式的结果上。  
返回非引用类型的函数，连同算术、关系、位以及后置的递增/递减运算符，都生成右值。我们不能将一个左值引用绑定到这类表达式上，但是我们可以将一个constå的左值引用(这样的话，该引用也就不可以修改引用的对象，毕竟右值不能修改)或者一个右值引用绑定到这类表达式上。  
### 左值持久；右值短暂
左值有持久的状态，而右值要么是字面常量，要么是在表达式求值过程中创建的临时对象。所以右值引用可以自由的接管所引用的对象的资源(因为右值是没有其他的用户在使用的)。  
虽然不能将一个右值引用直接绑定到一个左值上，但是我们可以显示地将一个左值转换为对应的右值引用类型，我们还可以通过std::move来获得绑定到左值上的右值引用。
```cpp
template<typename T> void f3(T&& t);
f3(42); //实参是一个int类型的右值;模板类型T是int类型, 模板参数t是类型int&&
f3(i);  //i是一个int类型的左值，T为int&，模板参数t是类型int&
```
这里判断T的类型也简单，先通过引用折叠知道了函数参数类型，然后T&&就是函数参数类型，就能推导出T的类型
### 万能引用和引用折叠
通常情况下，我们不能将一个右值引用绑定到一个左值上，但是C++在正常绑定规则之外定义了两个例外规则，允许这种绑定。这两个例外规则是move这种标准库设施正确工作的基础。
- 当我们将一个左值传递给函数的右值引用参数，且此右值引用指向模板参数时，编译器推断模板类型参数为实参的左值引用类型。因此我们调用f3(i)时，编译器推断T的类型问int&，而非int。看起来f3的函数参数是一个int&的右值引用。通常，我们不能(直接)定义一个引用的引用。但是通过类型别名或者通过模板类型参数间接定义是可以的。
- 如果我们间接创建一个引用的引用，则这些引用形成了“折叠”。在所有情况下(除了一个例外)，引用会折叠成一个普通的左值引用类型。在新标准中，折叠规则扩展到右值引用。只在一种特殊情况下引用会折叠成右值引用:右值引用的右值引用。即，对于一个给定类型X:
```
X& &、X& &&和X&& &都会折叠成类型X&
类型X&& &&折叠成x&&
```
引用折叠只能应用于间接创建的引用的引用，如类型别名或者模板参数  

这两个规则导致了两个重要结果:
- 如果一个函数参数是一个指向模板类型参数的右值引用(如，T&&)，则它可以被绑定到一个左值；且
- 如果实参是一个左值，则推断出来的模板实参类型将是一个左值引用，且函数参数将被实例化为一个(普通)左值引用参数(T&)

如果一个函数参数是指向模板类型参数的右值引用(如,`T&&`)，它对应的实参的const属性和左值/右值属性将得到保持。 
这里的T&&叫做万能引用(来自Effective modern C++的说法) 
## remove_reference
我们用`remove_reference`来获得元素类型。`remove_reference`模板有一个模板类型参数和一个名为type(public，remove_reference是个struct)的类型成员。如果我们用一个引用类型实例化`remove_reference`,则type表示被引用的类型，例如：`remove_reference<int&>::type`就是`int`。
```cpp
template<typename _Tp>
struct remove_reference
{ typedef _Tp     type; };

template<typename _Tp>
struct remove_reference<_Tp&>
{ typedef _Tp     type; };

template<typename _Tp>
struct remove_reference<_Tp&&>
{ typedef _Tp   type; };
```
## std::move
我们不能直接将一个右值引用绑定到一个左值上，但是`std::move`可以帮我们做到这件事，标准库是这样定义`std::move`的：
```cpp
template<typename T>
typename remove_reference<T>::type&& move(T&& t)
{
  return static_cast<typename remove_reference<T>::type&&>(t);
}
```
`std::move`是一个模板函数，它可以接受任何类型的实参，因为他的函数参数T&&是一个指向模板类型参数的右值引用。通过引用折叠，此参数可以与任何类型的实参(左值右值都可以)匹配。  
返回类型是模板参数T类型的右值引用，返回参数前面加上typename关键字是因为作用域运算符::后面跟的可能是类型或者是static数据成员，我们用typename来显示指示这里的`remove_reference<T>::type`是一个类型而不是static数据成员，`remove_reference`函数会去掉T的引用属性(如果T是引用类型会得到T所引用对象)。  
这里的左值往右值转换是由`static_cast`来完成的，虽然不能隐式地将一个左值转换成右值，但是我们可以用`static_cast`来显示的转换。

对于右值:
```cpp
std::move(string("aaa"));
```
经过模板参数推导后，模板参数t类型为string&&，模板类型T为string。remove_reference::type类型为string，返回类型为string&&，因此，对于右值类型string&&的参数，直接返回string&&右值类型。
对于左值:
```cpp
string s("aaa");
std::move(s);
```
经过模板参数推导后，模板参数t类型为string&, 模板类型T为string&，remove_reference::type类型为string，因为remove_reference会把引用属性去掉，返回string&&，所以传递给std::move左值类型的变量，std::move会返回右值类型。
## std::forward完美转发
看个例子:
```cpp
#include <utility>
#include <iostream>
class X{
    ...
};
void g(X&) {
    std::cout<< "g() for variable\n";
}

void g(X const&) {
    std::cout<< "g() for constant\n";
}

void g(X&&) {
    std::cout<< "g() for movable object\n";
}

void f(X& val) {
    g(val);     // call g(X&)
}

void f(X const& val) {
    g(val);     // call g(X const &)
}

void f(X&& val) {
    g(std::move(val));// val is non-const value lvalue => needs ::move to call g(X&&);
    //这里需要使用std::move，因为参数的移动语义不会被一起传递。
    //一切有名字的对象都是左值。
    //移动语义不会自动传递这一事实是有意且重要的。 如果不是，我们将在第一次在函数中使用可移动对象时失去它的值。 
    //The fact that move semantics is not automatically passed through is intentionaland important. 
    // If it weren’t, we would lose the value of a movable object the firsttime we use it in a function. 
}
int main()
{
    X v;
    X const c;
    f(v);               //f(X&) => g(X&)
    f(c);               //f(X const&) => g(X const&)
    f(X());             //f(X&&) => g(X&&)
    f(std::move(v));    //f(X&&) => g(X&&)
}
```
上面的代码繁琐了，是否可以简化， ok,使用模板来简化代码
```cpp
template<typename T>
void f(T val) {
    g(val);
}
```
但是这里还是那个问题，针对右值不适用，因为val的右值属性会消失。那么就和预期的结果不符合。正是因为参数在传递的过程中可能会丢失掉某些属性，所以这个时候需要使用std::forward来进行完美转发参数。那么上面的模板可以改成
```cpp
template<typename T>
void f(T&& val) {
    g(std::forward<T>(val));
}
```
下面是forward的实现原理
```cpp
template <class _Tp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
_Tp&& 
forward(typename remove_reference<_Tp>::type& __t) _NOEXCEPT
{
    return static_cast<_Tp&&>(__t);
}

template <class _Tp>
inline _LIBCPP_INLINE_VISIBILITY _LIBCPP_CONSTEXPR
_Tp&&
forward(typename remove_reference<_Tp>::type&& __t) _NOEXCEPT
{
    static_assert(!is_lvalue_reference<_Tp>::value,
                  "can not forward an rvalue as an lvalue");
    return static_cast<_Tp&&>(__t);
}

```
第一个函数用来转发左值，假如传入的参数是string&类型，那么_Tp是string&类型，返回string& &&类型，折叠后就是string&。  
第二个函数用来转发右值。假如传入的参数是string&&类型，那么_Tp的类型推断出来是string或者string&&，返回值也是string&&。 
与`std::move`不同，`std::forward`必须通过显式模板实参来调用。`std::forward`返回该显示实参类型的右值引用。即，`std::forward<T>`的返回类型是`T&&`。当用于一个指向模板参数类型的右值引用函数参数(`T&&`)时，`std::forward`会保持实参类型的所有细节。
```cpp
template<typename Type> void intermediary(Type &&arg)
{
  finalFcn(std::forward<Type>(arg));
}
```

例子中我们使用Type作为forward的显示模板实参类型，它是从arg推断出来的。由于arg是一个模板类型参数的右值引用，Type将表示传递给arg的实参的所有类型信息。如果实参是一个右值(如，`int&&`)，则Type是一个普通(非引用)类型`int`,forward<int>将返回`int&&`，如果实参是一个左值(如，`int&`，则通过引用折叠，Type本身是一个左值引用类型(`int&`)。在此情况下，返回类型是一个指向左值引用类型的右值引用。即`int&& &`，经过引用折叠后返回的类型就变成了`T&`。