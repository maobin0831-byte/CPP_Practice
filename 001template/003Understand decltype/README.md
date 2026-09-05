# Understand `decltype`

This example follows Item 3 of *Effective Modern C++*. It focuses on the
differences between `decltype` and ordinary template/`auto` deduction.

## Build and run

From this directory:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\decltype_demo.exe
```

With a single-configuration generator, run `build/decltype_demo` instead.

## Topics demonstrated

1. Applying `decltype` to names returns their declared type, while applying it
   to expressions reports the expression type.
2. An lvalue expression that is not an unparenthesized name produces `T&`.
   Therefore `decltype(x)` and `decltype((x))` can be different.
3. `decltype(auto)` uses `decltype` rules for variable and function return type
   deduction, preserving references when the expression requires it.
4. `authAndAccess` uses `decltype(auto)` and `std::forward` to return exactly
   what a container's `operator[]` returns, including proxy types such as
   `std::vector<bool>::reference`.

The program prints compiler-spelled types and uses `static_assert` for the
portable conclusions. The final access example also modifies a vector element,
which makes accidental return-by-value behavior visible immediately.
