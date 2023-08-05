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

add_library(ut INTERFACE
    vendor/boost/ut.hpp
)
target_include_directories(ut INTERFACE vendor/)
# MSVC is bogus with modules right now...
target_compile_definitions(ut INTERFACE BOOST_UT_DISABLE_MODULE)
target_compile_features(ut INTERFACE cxx_std_20)

add_executable(error-test
    src/go/error.test.cpp)
target_link_libraries(error-test PRIVATE go-error ut)

add_executable(error-string-test
    src/go/error_string.test.cpp)
target_link_libraries(error-string-test PRIVATE go-error ut)

add_executable(errorf-test
    src/go/errorf.test.cpp)
target_link_libraries(errorf-test PRIVATE go-error ut)

add_executable(error-code-test
    src/go/error_code.test.cpp)
target_link_libraries(error-code-test PRIVATE go-error ut)
