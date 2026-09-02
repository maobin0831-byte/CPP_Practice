#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// 一个很小的示例类，用来给本练习提供一个真实对象。
// 本文件的重点是“模板类型推导”，Widget 本身没有复杂逻辑。
class Widget {
public:
    // explicit：禁止把 std::string 隐式转换成 Widget。
    // std::move(name)：把构造函数形参所持有的字符串资源移动给成员 name_，
    // 避免再复制一次字符串内容。
    explicit Widget(std::string name) : name_(std::move(name)) {}

    // 返回 const 引用，调用者可以读取名字，但不能通过该引用修改 name_。
    // [[nodiscard]] 提醒调用者不要无意中丢弃返回值；noexcept 表示不会抛异常。
    [[nodiscard]] const std::string& name() const noexcept {
        return name_;
    }

private:
    std::string name_;
};

// 将模板实参 T 转换为一段可打印的类型名称。
//
// C++ 标准库没有一个可以直接得到“完整类型名称”的通用接口，因此这里利用各编译器
// 提供的函数签名字符串：MSVC 使用 __FUNCSIG__，Clang/GCC 使用
// __PRETTY_FUNCTION__。当编译器实例化 type_name<int>() 时，签名字符串中会包含
// int；下面的代码从完整签名中截取出 T 对应的部分。
//
// constexpr 使这段查找和截取可以在编译期完成，返回的 string_view 只引用编译器
// 提供的静态字符串，不负责内存所有权。
template <typename T>
constexpr std::string_view type_name() noexcept {
#if defined(_MSC_VER)
    // MSVC 示例签名：
    // class std::basic_string_view<...> __cdecl type_name<int>(void) noexcept
    constexpr std::string_view signature = __FUNCSIG__;
    constexpr std::string_view prefix = "type_name<";
    constexpr std::string_view suffix = ">(void) noexcept";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(suffix, begin);
    return signature.substr(begin, end - begin);
#elif defined(__clang__)
    // Clang 的签名中包含类似“[T = int]”的片段。
    constexpr std::string_view signature = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(']', begin);
    return signature.substr(begin, end - begin);
#elif defined(__GNUC__)
    // GCC 的签名中也包含“T = int”，并可能在后面跟随分号和其他别名信息。
    constexpr std::string_view signature = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(';', begin);
    return signature.substr(begin, end - begin);
#else
    return "(compiler type spelling unavailable)";
#endif
}

// 打印转发引用示例中的两个关键类型：
// 1. T：模板推导真正得到的类型；
// 2. T&&：函数形参最终使用的类型。
//
// parameter 没有参与计算，所以用 (void)parameter 明确表示“有意不使用”，避免警告。
template <typename T>
void print_forwarding_types(std::string_view parameter_form,
                            std::string_view label, T&& parameter) {
    (void)parameter;
    std::cout << "  " << label << " [" << parameter_form << "]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<T&&>() << '\n';
}

// 情况 1：形参是普通左值引用 T&。
//
// 推导时先忽略实参自身的“引用性”，再让 T 匹配其余类型：
// - int 左值          -> T = int，形参 = int&
// - const int 左值    -> T = const int，形参 = const int&
// - const int& 类型变量 -> 仍是 T = const int；实参的引用本身不会进入 T
template <typename T>
void lvalue_reference_demo(T& param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [T&]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 形参是 const T& 时，形参中的 const 已经由模板声明给出，不需要推导进 T。
// 因此无论传入 int 左值还是 const int 左值，这几个示例通常都会得到 T = int，
// 最终形参类型均为 const int&。
template <typename T>
void const_reference_demo(const T& param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [const T&]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 指针形式 T* 的推导规则与普通引用类似：只匹配“指针所指对象的类型”。
// int* 会得到 T = int；const int* 会得到 T = const int。
// 注意，这里的 const 修饰被指向的 int，所以它属于底层 const，必须保留。
template <typename T>
void pointer_demo(T* param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [T*]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 情况 2：当 T 需要由调用实参推导时，T&& 是“转发引用”（旧称万能引用）。
//
// 它有一条特殊规则：
// - 传左值 int          -> T = int&
// - 传左值 const int    -> T = const int&
// - 传右值 int          -> T = int
//
// 随后应用引用折叠规则：T 是 int& 时，T&& 即 int& &&，最终折叠成 int&；
// T 是 int 时，T&& 仍为 int&&。因此转发引用能够区分左值和右值。
template <typename T>
void forwarding_reference_demo(T&& param, std::string_view label) {
    // 命名变量 param 在表达式中永远是左值，即使它的声明类型是 T&&。
    // std::forward<T>(param) 根据 T 恢复调用者原来的值类别：左值仍为左值，
    // 右值重新成为右值。这就是“完美转发”的核心。
    print_forwarding_types("T&& (forwarding reference)", label,
                           std::forward<T>(param));
}

// 情况 3：形参按值接收，即形式为 T。
//
// 函数会创建一份独立的 param，因此实参最外层（顶层）的引用、const、volatile
// 都不会影响这份副本，会在推导时被去掉。例如 const int& 最终得到 T = int。
// 但指针指向对象的 const 属于“底层 const”，不能去掉：const char* 仍会保留。
template <typename T>
void by_value_demo(T param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [T]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 接受“对含 N 个元素的数组的引用”。
// T 是数组元素类型，N 是编译器从实参数组长度直接推导出的非类型模板参数。
// 因为形参使用引用，数组不会退化成指针，所以可以安全地在编译期获得长度。
template <typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept {
    return N;
}

// 用于演示：函数作为实参时，按值传递会退化为函数指针，按引用传递则保留函数类型。
void someFunc(int, double) {}

template <typename T>
void function_by_value_demo(T param) {
    (void)param;
    std::cout << "  function passed by value [T]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 使用 T& 接住函数，因而不会发生“函数到函数指针”的退化。
template <typename T>
void function_by_reference_demo(T& param) {
    (void)param;
    std::cout << "  function passed by reference [T&]\n"
              << "    T                 = " << type_name<T>() << '\n'
              << "    decltype(param)    = " << type_name<decltype(param)>() << '\n';
}

// 仅用于分隔控制台输出中的各组实验。
void print_header(std::string_view title) {
    std::cout << "\n--- " << title << " ---\n";
}

int main() {
    // const auto 让编译器从右侧推导变量类型，并给变量加上顶层 const。
    const auto widget = Widget{"Effective Modern C++"};
    std::cout << "Practice target: " << widget.name() << '\n';

    // x  ：int
    // cx ：const int
    // rx ：const int&，引用 x，但只能通过 rx 读取，不能修改 x
    int x = 27;
    const int cx = x;
    const int& rx = x;

    print_header("1. T&: ordinary reference");
    // 依次观察普通 int、const int，以及引用类型变量对 T& 推导的影响。
    lvalue_reference_demo(x, "x (int lvalue)");
    lvalue_reference_demo(cx, "cx (const int lvalue)");
    lvalue_reference_demo(rx, "rx (const int&; reference ignored)");

    print_header("2. const T&: reference-to-const");
    // const 是形参模板 const T& 自带的，所以不会被推导进 T。
    const_reference_demo(x, "x");
    const_reference_demo(cx, "cx");
    const_reference_demo(rx, "rx");

    print_header("3. T*: pointer deduction");
    // &x 是 int*；px 是 const int*。后者的底层 const 会被保留在 T 中。
    pointer_demo(&x, "&x (int*)");
    const int* px = &x;
    pointer_demo(px, "px (const int*)");

    print_header("4. T&&: forwarding reference");
    // 有名字的变量 x/cx/rx 都是左值；字面量 27 是纯右值。
    // 对前三项，T 会被推导为某种左值引用；对 27，T 被推导为 int。
    forwarding_reference_demo(x, "x lvalue");
    forwarding_reference_demo(cx, "cx const lvalue");
    forwarding_reference_demo(rx, "rx const lvalue");
    forwarding_reference_demo(27, "27 rvalue");

    print_header("5. T: pass by value");
    // 按值传递会复制出新对象，故最外层的 const/volatile/引用全部被去掉。
    by_value_demo(x, "x");
    by_value_demo(cx, "cx (top-level const removed)");
    by_value_demo(rx, "rx (reference removed)");
    volatile int vx = x;
    by_value_demo(vx, "vx (top-level volatile removed)");
    // 从右向左读：ptr 是“const 指针”，指向“const char”。
    // 按值推导会去掉指针自身的顶层 const，但保留 char 的底层 const，
    // 所以 T 最终是 const char*。
    const char* const ptr = "Fun with pointers";
    by_value_demo(ptr, "const pointer to const char");

    print_header("6. Array arguments and arraySize");
    // name 的真实类型是 const char[13]（包含字符串末尾的 '\0'）。
    // 按值传给模板时，数组退化成 const char*；按引用传递则保留完整数组类型。
    const char name[] = "J. P. Briggs";
    by_value_demo(name, "name passed by value (array decays to pointer)");
    lvalue_reference_demo(name, "name passed by reference (array preserved)");

    // arraySize(keyVals) 在编译期推导出 N = 7，因此它既能用于输出，
    // 也能作为 std::array 的模板参数。mappedVals{} 使用零值初始化，元素全为 0。
    int keyVals[] = {1, 3, 7, 9, 11, 22, 35};
    std::array<int, arraySize(keyVals)> mappedVals{};
    std::cout << "  arraySize(keyVals)   = " << arraySize(keyVals) << '\n'
              << "  std::array size       = " << mappedVals.size() << '\n';
    // static_assert 是编译期断言：若长度不是 7，程序会直接编译失败。
    static_assert(arraySize(keyVals) == 7);

    print_header("7. Function arguments");
    // 按值：someFunc 退化为 void (*)(int, double)，即函数指针。
    // 按引用：保留 void(int, double) 这一函数类型，形参则是该函数的左值引用。
    function_by_value_demo(someFunc);
    function_by_reference_demo(someFunc);

    std::cout << "\nThe examples show: references are ignored for deduction, forwarding\n"
                 "references distinguish lvalues/rvalues, by-value drops top-level\n"
                 "const/volatile, and arrays/functions decay unless bound to a reference.\n";
    return 0;
}
