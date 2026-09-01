# C++ 模板类型推导学习笔记

本项目通过代码示例练习 C++ 模板函数的类型推导规则，重点比较 `T&`、`T&&` 和 `T` 三种形参写法。

示例源码位于：

`001template/Understand template type deduction/src/main.cpp`

源码中可以直接运行并打印编译器推导出的 `T` 和形参类型，适合对照下面的规则阅读。

## 把三种 Case 放在一起比较

### Case 1：`T&`，普通左值引用

函数模板写法：

```cpp
template <typename T>
void f(T& param);
```

核心规则：

- 实参自身的引用属性不参与 `T` 的推导；
- `const` 会保留在 `T` 中；
- 数组传给引用形参时不会退化成指针；
- 该形参主要接收左值，不能直接绑定普通右值。

例如：

```cpp
const int value = 27;
f(value);
```

推导过程：

```text
实参类型：const int
        ↓
T = const int
        ↓
param = const int&
```

如果实参本身声明为 `const int&`，这里的 `&` 仍然不会额外进入 `T`：

```cpp
int number = 27;
const int& reference = number;
f(reference);
```

依然可以理解为：

```text
T = const int
param = const int&
```

数组也会保留原始类型。假设：

```cpp
const char name[] = "J. P. Briggs";
f(name);
```

使用 `T&` 时，`name` 不会先转换成 `const char*`，而是保留为类似 `const char[13]` 的数组类型。

### Case 2：`T&&`，转发引用

函数模板写法：

```cpp
template <typename T>
void f(T&& param);
```

当 `T` 需要通过调用实参推导时，`T&&` 是转发引用，也常被称为万能引用。它的“万能”不是因为 `&&` 本身，而是因为模板类型推导和引用折叠一起作用。

核心规则：

- 左值实参：`T` 会被推导成左值引用类型；
- 右值实参：`T` 按普通规则推导，不会自动变成引用；
- 最终的 `T&&` 会根据引用折叠规则得到真正的形参类型。

#### 传入左值

```cpp
int x = 27;
f(x);
```

推导过程：

```text
x 是左值
   ↓
T = int&
   ↓
T&& = int& &&
   ↓
引用折叠后：int&
```

`const` 左值也会保留：

```cpp
const int cx = 27;
f(cx);
```

结果可以写成：

```text
T = const int&
T&& 折叠为 const int&
```

#### 传入右值

```cpp
f(27);
```

推导过程：

```text
27 是右值
   ↓
T = int
   ↓
T&& = int&&
```

因此同一个 `T&&` 模板既能接收左值，也能接收右值，同时还能保留调用者原本的值类别。函数内部的命名形参虽然声明为 `T&&`，但只要它有名字，在表达式中就会是左值；需要恢复原始值类别时，应使用：

```cpp
std::forward<T>(param)
```

这就是完美转发的基本用法。

### Case 3：`T`，按值传递

函数模板写法：

```cpp
template <typename T>
void f(T param);
```

因为函数会根据实参创建一个独立的副本，所以推导时通常会丢弃以下属性：

- 引用：忽略 `&`；
- 顶层 `const`：忽略；
- 顶层 `volatile`：忽略；
- 数组：退化成指针；
- 函数：退化成函数指针。

例如：

```cpp
const int value = 27;
const int& reference = value;
volatile int flag = 0;
```

按值传递时可以理解为：

```text
const int       → T = int
const int&      → T = int
volatile int    → T = int
```

数组按值传递会退化：

```cpp
int values[] = {1, 3, 7};
f(values);
```

推导结果类似于：

```text
int[3] → int*
T = int*
```

函数按值传递也会退化为函数指针：

```cpp
void someFunc(int, double);
f(someFunc);
```

推导结果类似于：

```text
void(int, double) → void (*)(int, double)
T = void (*)(int, double)
```

需要注意：按值推导去掉的是指针自身的顶层 `const`，不会去掉指向对象的底层 `const`。

```cpp
const char* const ptr = "hello";
f(ptr);
```

这里的 `ptr` 是“指向 `const char` 的 `const` 指针”。按值传递会去掉指针自身的 `const`，但保留指向字符的 `const`，所以 `T` 仍类似于 `const char*`。

## 三种写法快速对照

| 形参写法 | 左值实参 | 右值实参 | `const` | 数组 | 函数 |
| --- | --- | --- | --- | --- | --- |
| `T&` | 可以接收 | 通常不能接收 | 保留 | 不退化 | 不退化 |
| `T&&` | `T` 变成引用 | `T` 按值推导 | 保留 | 可保留 | 可保留 |
| `T` | 复制一份 | 复制一份 | 顶层属性丢弃 | 退化为指针 | 退化为函数指针 |

## 最终口诀

可以直接记住下面四句话：

> ① `T&`：保留 `const`，不让数组退化。
>
> ② `T&&`：左值特殊处理，`T` 会变成引用；右值正常处理。
>
> ③ `T`：按值复制，所以 `const`、`volatile` 和引用属性通常被忽略。
>
> ④ 数组和函数：按值传递会退化成指针；传给引用则可以保留原始类型。

最值得反复看的组合是：

```text
T&       → const 保留
T&&      → 左值时 T 变成引用
T        → 顶层 const 丢掉
T        → 数组退化成指针
T&       → 数组不退化
```

## 和 `std::move`、右值引用的联系

`std::move` 本身不会移动对象，它只是把表达式转换成右值，从而允许后续代码选择移动构造或移动赋值。

而 `T&&` 在模板中是否是转发引用，要看它是否满足“未被其他限定、并且 `T` 需要推导”这个条件。模板类型推导先决定 `T`，引用折叠再决定最终形参类型，所以才会出现：

```text
左值 → T = 类型& → T&& 折叠为 类型&
右值 → T = 类型  → T&& 保持为 类型&&
```

这几条规则串起来，就是本练习关于模板类型推导的核心。
