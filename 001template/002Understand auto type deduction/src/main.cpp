#include <initializer_list>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// A small compile-time type spelling helper. It is only used for learning output.
template <typename T>
constexpr std::string_view type_name() noexcept {
#if defined(_MSC_VER)
    constexpr std::string_view signature = __FUNCSIG__;
    constexpr std::string_view prefix = "type_name<";
    constexpr std::string_view suffix = ">(void) noexcept";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(suffix, begin);
    return signature.substr(begin, end - begin);
#elif defined(__clang__)
    constexpr std::string_view signature = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(']', begin);
    return signature.substr(begin, end - begin);
#elif defined(__GNUC__)
    constexpr std::string_view signature = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    const auto begin = signature.find(prefix) + prefix.size();
    const auto end = signature.find(';', begin);
    return signature.substr(begin, end - begin);
#else
    return "(compiler type spelling unavailable)";
#endif
}

void print_type(std::string_view label, std::string_view spelling) {
    std::cout << "  " << label << " = " << spelling << '\n';
}

template <typename T>
void print_declared_type(std::string_view label) {
    print_type(label, type_name<T>());
}

// These functions are the conceptual templates behind auto declarations:
// auto is T, and the rest of the declaration is ParamType.
template <typename T>
void equivalent_by_value(T param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [T param]\n"
              << "    T              = " << type_name<T>() << '\n'
              << "    decltype(param) = " << type_name<decltype(param)>() << '\n';
}

template <typename T>
void equivalent_const_reference(const T& param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [const T& param]\n"
              << "    T              = " << type_name<T>() << '\n'
              << "    decltype(param) = " << type_name<decltype(param)>() << '\n';
}

template <typename T>
void equivalent_forwarding_reference(T&& param, std::string_view label) {
    (void)param;
    std::cout << "  " << label << " [T&& param]\n"
              << "    T              = " << type_name<T>() << '\n'
              << "    decltype(param) = " << type_name<decltype(param)>() << '\n';
}

template <typename T>
void print_forwarded_type(T&& param, std::string_view label) {
    std::cout << "  " << label << " via std::forward<T>(param) = "
              << type_name<decltype(std::forward<T>(param))>() << '\n';
}

void section(std::string_view title) {
    std::cout << "\n--- " << title << " ---\n";
}

void some_function(int, double) {}

auto create_value() {
    return 42; // Return type deduction here follows template deduction.
}

int main() {
    section("1. auto maps to template type deduction");

    int x = 27;
    const int cx = x;
    const int& rx = x;

    auto value = 27;
    const auto const_value = x;
    const auto& reference = x;

    print_declared_type<decltype(value)>("auto value = 27");
    print_declared_type<decltype(const_value)>("const auto const_value = x");
    print_declared_type<decltype(reference)>("const auto& reference = x");

    static_assert(std::is_same_v<decltype(value), int>);
    static_assert(std::is_same_v<decltype(const_value), const int>);
    static_assert(std::is_same_v<decltype(reference), const int&>);

    equivalent_by_value(x, "x");
    equivalent_by_value(rx, "rx (reference ignored)");
    equivalent_const_reference(x, "x");
    (void)reference;

    section("2. auto&& is a forwarding reference");
    auto&& uref1 = x;
    auto&& uref2 = cx;
    auto&& uref3 = 27;

    print_declared_type<decltype(uref1)>("auto&& uref1 = x");
    print_declared_type<decltype(uref2)>("auto&& uref2 = cx");
    print_declared_type<decltype(uref3)>("auto&& uref3 = 27");

    static_assert(std::is_same_v<decltype(uref1), int&>);
    static_assert(std::is_same_v<decltype(uref2), const int&>);
    static_assert(std::is_same_v<decltype(uref3), int&&>);
    (void)uref1;
    (void)uref2;
    (void)uref3;

    equivalent_forwarding_reference(x, "x lvalue");
    equivalent_forwarding_reference(cx, "cx const lvalue");
    equivalent_forwarding_reference(27, "27 rvalue");
    print_forwarded_type(x, "x lvalue");
    print_forwarded_type(27, "27 rvalue");

    section("3. arrays and functions: decay depends on the declaration");
    const char name[] = "R. N. Briggs";
    auto array_pointer = name;
    auto& array_reference = name;

    print_declared_type<decltype(array_pointer)>("auto array_pointer = name");
    print_declared_type<decltype(array_reference)>("auto& array_reference = name");
    static_assert(std::is_same_v<decltype(array_pointer), const char*>);
    static_assert(std::is_same_v<decltype(array_reference), const char (&)[13]>);
    (void)array_pointer;
    (void)array_reference;

    auto function_pointer = some_function;
    auto& function_reference = some_function;
    print_declared_type<decltype(function_pointer)>("auto function_pointer = some_function");
    print_declared_type<decltype(function_reference)>("auto& function_reference = some_function");
    static_assert(std::is_same_v<decltype(function_pointer), void (*)(int, double)>);
    static_assert(std::is_same_v<decltype(function_reference), void (&)(int, double)>);
    (void)function_pointer;
    (void)function_reference;

    section("4. braced initializers are auto's special case");
    auto copy_initialized = 27;
    auto direct_initialized(27);
    auto copy_list = {27};
    auto direct_list{27};

    print_declared_type<decltype(copy_initialized)>("auto copy_initialized = 27");
    print_declared_type<decltype(direct_initialized)>("auto direct_initialized(27)");
    print_declared_type<decltype(copy_list)>("auto copy_list = {27}");
    print_declared_type<decltype(direct_list)>("auto direct_list{27}");
    static_assert(std::is_same_v<decltype(copy_initialized), int>);
    static_assert(std::is_same_v<decltype(direct_initialized), int>);
    static_assert(std::is_same_v<decltype(copy_list), std::initializer_list<int>>);
    static_assert(std::is_same_v<decltype(direct_list), int>);
    (void)copy_initialized;
    (void)direct_initialized;
    (void)copy_list;
    (void)direct_list;

    const auto numbers = {1, 2, 3};
    std::cout << "  initializer_list size = " << numbers.size() << '\n';
    std::cout << "  auto mixed = {1, 2, 3.0} is intentionally omitted: "
                 "initializer_list element deduction would fail.\n";

    section("5. auto in return types and lambda parameters");
    const auto answer = create_value();
    print_declared_type<decltype(answer)>("create_value() result");
    static_assert(std::is_same_v<decltype(answer), const int>);
    (void)answer;

    std::vector<int> values;
    auto assign = [&values](const auto& new_value) { values = new_value; };
    assign(std::vector<int>{1, 2, 3});
    std::cout << "  generic lambda assigned " << values.size() << " values\n";
    std::cout << "  assign({1, 2, 3}) is intentionally omitted: lambda parameter\n"
                 "  deduction follows template rules and cannot deduce a braced list.\n";

    std::cout << "\nKey rule: auto usually follows template deduction; only a variable\n"
                 "initialized with braces gets the special initializer_list treatment.\n";
    return 0;
}
