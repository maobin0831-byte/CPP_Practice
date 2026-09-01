cmake_minimum_required(VERSION 3.25)

project(EffectiveModernCpp
    VERSION 0.1.0
    DESCRIPTION "Examples and experiments for Effective Modern C++"
    LANGUAGES CXX
)

if(NOT MSVC)
    message(FATAL_ERROR
        "This project requires the MSVC compiler. Configure it with a Visual Studio generator."
    )
endif()

add_executable(effective_modern_cpp
    src/main.cpp
)

target_compile_features(effective_modern_cpp PRIVATE cxx_std_20)

set_target_properties(effective_modern_cpp PROPERTIES
    CXX_EXTENSIONS OFF
)

target_compile_options(effective_modern_cpp PRIVATE
    /W4
    /permissive-
    /EHsc
    /utf-8
    /Zc:__cplusplus
    /diagnostics:caret
)

target_compile_definitions(effective_modern_cpp PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT effective_modern_cpp)
