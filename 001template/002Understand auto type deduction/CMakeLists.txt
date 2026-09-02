cmake_minimum_required(VERSION 3.25)

project(AutoTypeDeduction
    VERSION 0.1.0
    DESCRIPTION "Examples and experiments for Effective Modern C++ Item 2"
    LANGUAGES CXX
)

add_executable(auto_type_deduction
    src/main.cpp
)

target_compile_features(auto_type_deduction PRIVATE cxx_std_20)

set_target_properties(auto_type_deduction PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(auto_type_deduction PRIVATE
        /W4
        /permissive-
        /EHsc
        /utf-8
        /Zc:__cplusplus
        /diagnostics:caret
    )

    target_compile_definitions(auto_type_deduction PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
else()
    target_compile_options(auto_type_deduction PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT auto_type_deduction)
