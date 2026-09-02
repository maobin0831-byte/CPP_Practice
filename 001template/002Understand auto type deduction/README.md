# 理解 `auto` 类型推导

本练习对应《Effective Modern C++》Item 2，使用可以直接构建和运行的 C++ 工程，把 `auto` 类型推导和 `001Understand template type deduction` 中的规则逐一对照。

## 工程结构

```text
002Understand auto type deduction/
├─ CMakeLists.txt
├─ build.bat
├─ README.md
└─ src/main.cpp
```

在当前目录执行：

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\auto_type_deduction.exe
```

上面最后一行适用于 Visual Studio 生成器；单配置的 GCC/Clang 生成器通常运行 `./build/auto_type_deduction`。项目也兼容 GCC/Clang，非 MSVC 编译器会启用等价的警告选项。

## 学习要点

### 1. `auto` 是模板推导的直接映射

声明中的 `auto` 对应函数模板里的 `T`，其余类型说明符对应 `ParamType`：

```cpp
auto x = 27;             // 类似 template<typename T> void f(T param)
const auto cx = x;       // 类似 template<typename T> void f(const T param)
const auto& rx = x;      // 类似 template<typename T> void f(const T& param)
```

`main.cpp` 中的 `equivalent_*` 函数会打印模板版本的 `T` 和形参类型，并通过 `static_assert` 检查变量声明的最终类型。

### 2. 三种常见说明符

- `auto`（既不是指针也不是引用）按值推导，顶层 `const`、`volatile` 和引用会被忽略。
- `const auto&` 保留对对象的只读引用，不发生数组或函数到指针的退化。
- `auto&&` 在变量初始化中是转发引用：左值推导为左值引用，右值推导为右值引用。

变量名本身是左值；如果在模板代码中需要恢复调用方的值类别，应使用 `std::forward<T>`。

### 3. 数组和函数

`auto array_pointer = name`、`auto function_pointer = some_function` 会发生数组/函数退化；加上 `&` 则保留完整的数组或函数类型。示例用 `static_assert` 固化了 `const char[13]`、函数指针和函数引用的结论。

### 4. 花括号初始化的唯一例外

变量使用 `auto` 且采用拷贝列表初始化（`=` 加花括号）时，类型会按 `std::initializer_list<T>` 推导：

```cpp
auto a = {27};   // std::initializer_list<int>
auto b{27};      // int：C++20 中单元素直接列表初始化
```

直接列表初始化 `auto b{27}` 在当前 C++ 标准中要求单个元素，并直接推导该元素类型；这也是它和 `auto a = {27}` 的关键区别。

`auto mixed = {1, 2, 3.0}` 会被拒绝，因为 `std::initializer_list<T>` 要求所有元素推导出同一个 `T`。这条失败示例没有放进可执行代码，以保持工程能够完整构建。

### 5. 函数返回值和泛型 lambda

C++14 开始，函数返回类型写成 `auto`、lambda 参数写成 `auto` 时，使用的是模板类型推导规则，而不是变量初始化的 `auto` 特例。因此下面两种写法都不能直接接收花括号列表：

```cpp
auto create_init_list() { return {1, 2, 3}; } // 不能推导返回类型
assign({1, 2, 3});                             // lambda 参数无法推导
```

可执行示例改用显式的 `std::vector<int>`，并展示泛型 lambda 的正常调用。

## 运行输出

程序会打印每个声明的编译器类型名称。不同编译器对数组引用、函数类型的显示格式可能略有差异，但 `static_assert` 验证的是标准规定的类型，才是本练习的重点。

## 记忆口诀

> `auto` 通常就是模板推导；变量采用拷贝列表初始化时，才有 `initializer_list` 特例。
>
> 函数返回值和 lambda 参数中的 `auto` 仍然遵循模板推导，不能靠花括号列表猜出类型。
