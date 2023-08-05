cmake_minimum_required (VERSION 3.18)
project(go-error CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(BUILD_TESTING ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 17)

include(Testing)

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

if (${BUILD_TESTING})
    add_our_test(error)
    target_sources(error-test PUBLIC src/go/error.test.cpp)
    target_link_libraries(error-test PRIVATE go-error)

    add_our_test(error-string)
    target_sources(error-string-test PUBLIC src/go/error_string.test.cpp)
    target_link_libraries(error-string-test PRIVATE go-error)

    add_our_test(errorf)
    target_sources(errorf-test PUBLIC src/go/errorf.test.cpp)
    target_link_libraries(errorf-test PRIVATE go-error)

    add_our_test(error-code)
    target_sources(error-code-test PUBLIC src/go/error_code.test.cpp)
    target_link_libraries(error-code-test PRIVATE go-error)

    add_our_test(error-cast)
    target_sources(error-cast-test PUBLIC src/go/error_cast.test.cpp)
    target_link_libraries(error-cast-test PRIVATE go-error)
endif()
