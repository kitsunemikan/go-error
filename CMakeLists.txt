cmake_minimum_required (VERSION 3.18)
project(go-error CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 17)

add_library(go-error STATIC)
target_sources(go-error PRIVATE
    src/go/error.hpp
    src/go/error.cpp
    src/go/error_string.hpp
    src/go/error_code.hpp
    src/go/error_cast.hpp
    src/go/errorf.hpp
)
target_include_directories(go-error PUBLIC
    src/
)

add_executable(go-error-test)
target_sources(go-error-test PRIVATE
    src/go-error-test.main.cpp
)
target_link_libraries(go-error-test PRIVATE go-error)
