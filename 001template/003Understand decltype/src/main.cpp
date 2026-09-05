#include <cstddef>
#include <deque>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

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

void section(std::string_view title) {
    std::cout << "\n--- " << title << " ---\n";
}

template <typename T>
void print_type(std::string_view label) {
    std::cout << "  " << label << " = " << type_name<T>() << '\n';
}

struct Widget {};

bool is_valid(const Widget&) {
    return true;
}

struct Point {
    int x;
    int y;
};

template <typename Container, typename Index>
decltype(auto) authAndAccess(Container&& container, Index index) {
    // A real authentication step would happen here in production code.
    return std::forward<Container>(container)[index];
}

decltype(auto) return_by_value(int value) {
    return value;
}

decltype(auto) return_lvalue(int& value) {
    return (value);
}

int main() {
    section("1. Names and ordinary expressions");
    const int i = 0;
    Widget widget;
    Point point{};
    std::vector<int> values{10, 20, 30};

    print_type<decltype(i)>("decltype(i)");
    print_type<decltype(widget)>("decltype(widget)");
    print_type<decltype(is_valid)>("decltype(is_valid)");
    print_type<decltype(Point::x)>("decltype(Point::x)");
    print_type<decltype(is_valid(widget))>("decltype(is_valid(widget))");
    print_type<decltype(values[0])>("decltype(values[0])");

    static_assert(std::is_same_v<decltype(i), const int>);
    static_assert(std::is_same_v<decltype(widget), Widget>);
    static_assert(std::is_same_v<decltype(is_valid), bool(const Widget&)>);
    static_assert(std::is_same_v<decltype(Point::x), int>);
    static_assert(std::is_same_v<decltype(is_valid(widget)), bool>);
    static_assert(std::is_same_v<decltype(values[0]), int&>);
    (void)point;

    section("2. The parenthesized-name special case");
    int number = 27;
    print_type<decltype(number)>("decltype(number)");
    print_type<decltype((number))>("decltype((number))");
    static_assert(std::is_same_v<decltype(number), int>);
    static_assert(std::is_same_v<decltype((number)), int&>);
    std::cout << "  number before authAndAccess = " << number << '\n';

    section("3. decltype(auto) preserves decltype rules");
    const Widget& widget_reference = widget;
    auto copied_widget = widget_reference;
    decltype(auto) named_widget_reference = widget_reference;
    decltype(auto) exact_widget_reference = (widget_reference);

    print_type<decltype(copied_widget)>("auto copied_widget = widget_reference");
    print_type<decltype(named_widget_reference)>(
        "decltype(auto) named_widget_reference = widget_reference");
    print_type<decltype(exact_widget_reference)>(
        "decltype(auto) exact_widget_reference = (widget_reference)");
    static_assert(std::is_same_v<decltype(copied_widget), Widget>);
    static_assert(std::is_same_v<decltype(named_widget_reference), const Widget&>);
    static_assert(std::is_same_v<decltype(exact_widget_reference), const Widget&>);

    print_type<decltype(return_by_value(1))>("return_by_value(1)");
    print_type<decltype(return_lvalue(number))>("return_lvalue(number)");
    static_assert(std::is_same_v<decltype(return_by_value(1)), int>);
    static_assert(std::is_same_v<decltype(return_lvalue(number)), int&>);

    section("4. authAndAccess: return exactly what operator[] returns");
    std::deque<int> deque{1, 2, 3};
    authAndAccess(deque, 1) = 42;
    std::cout << "  deque[1] after authAndAccess(deque, 1) = " << deque[1] << '\n';
    static_assert(std::is_same_v<decltype(authAndAccess(deque, 1)), int&>);

    const std::deque<int> const_deque{4, 5, 6};
    print_type<decltype(authAndAccess(const_deque, 1))>(
        "authAndAccess(const_deque, 1)");
    static_assert(std::is_same_v<decltype(authAndAccess(const_deque, 1)), const int&>);

    // A temporary can be read and copied before its lifetime ends. Keeping the
    // returned reference after this full expression would be a dangling reference.
    const auto copied = authAndAccess(std::deque<std::string>{"temporary"}, 0);
    std::cout << "  copied element from temporary deque = " << copied << '\n';

    std::vector<bool> flags{false, false};
    authAndAccess(flags, 0) = true;
    std::cout << "  vector<bool>[0] after proxy assignment = " << std::boolalpha
              << flags[0] << '\n';
    static_assert(std::is_same_v<decltype(authAndAccess(flags, 0)),
                                 std::vector<bool>::reference>);

    std::cout << "\nKey rule: decltype normally reports the exact type. For an lvalue\n"
                 "expression other than an unparenthesized name, it reports T&, and\n"
                 "decltype(auto) preserves that result.\n";
    return 0;
}
