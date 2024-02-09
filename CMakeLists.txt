cmake_minimum_required (VERSION 3.18)
project(go-error CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 17)

set(GOERROR_BUILD_TESTING ON CACHE BOOL "")
set(GOERROR_BUILD_EXAMPLES ON CACHE BOOL "")
set(GOERROR_BUILD_DOCS ON CACHE BOOL "")

include(Testing)

add_library(go-error STATIC)
target_sources(go-error PRIVATE
    src/go/go_error.hpp
    src/go/error.hpp
    src/go/error.cpp
    src/go/error_string.hpp
    src/go/error_code.hpp
    src/go/error_cast.hpp
    src/go/errorf.hpp
    src/go/wrap.hpp
)
target_include_directories(go-error PUBLIC
    src/
)

if (${GOERROR_BUILD_TESTING})
    add_our_test(error)
    target_sources(test-error PUBLIC src/go/error.test.cpp)
    target_link_libraries(test-error PRIVATE go-error)

    add_our_test(error-string)
    target_sources(test-error-string PUBLIC src/go/error_string.test.cpp)
    target_link_libraries(test-error-string PRIVATE go-error)

    add_our_test(errorf)
    target_sources(test-errorf PUBLIC src/go/errorf.test.cpp)
    target_link_libraries(test-errorf PRIVATE go-error)

    add_our_test(error-code)
    target_sources(test-error-code PUBLIC src/go/error_code.test.cpp)
    target_link_libraries(test-error-code PRIVATE go-error)

    add_our_test(error-cast)
    target_sources(test-error-cast PUBLIC src/go/error_cast.test.cpp)
    target_link_libraries(test-error-cast PRIVATE go-error)

    add_our_test(wrap)
    target_sources(test-wrap PUBLIC src/go/wrap.test.cpp)
    target_link_libraries(test-wrap PRIVATE go-error)

    add_executable(example-custom-error)
    target_sources(example-custom-error PUBLIC _examples/example_custom_error.main.cpp)
    target_link_libraries(example-custom-error PRIVATE go-error)
    add_test_suite_dependency(example-custom-error)
endif()

if (${GOERROR_BUILD_DOCS})
    find_package(doxygen)

    set(DOXYGEN_EXCLUDE_PATTERNS *.test.cpp)

    doxygen_add_docs(
        doxygen
        ALL
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating documentation"
    )
endif()
