cmake_minimum_required(VERSION 3.25)

project(DecltypeDemo
    VERSION 0.1.0
    DESCRIPTION "Examples and experiments for Effective Modern C++ Item 3"
    LANGUAGES CXX
)

add_executable(decltype_demo
    src/main.cpp
)

target_compile_features(decltype_demo PRIVATE cxx_std_20)

set_target_properties(decltype_demo PROPERTIES
    CXX_EXTENSIONS OFF
)

if(MSVC)
    target_compile_options(decltype_demo PRIVATE
        /W4
        /permissive-
        /EHsc
        /utf-8
        /Zc:__cplusplus
        /diagnostics:caret
    )

    target_compile_definitions(decltype_demo PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
else()
    target_compile_options(decltype_demo PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT decltype_demo)
