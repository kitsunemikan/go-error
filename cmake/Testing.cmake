## Find GoogleTest
if(${BUILD_TESTING})
    include(CTest)

    add_library(ut INTERFACE
        ${CMAKE_SOURCE_DIR}/vendor/boost/ut.hpp
    )
    target_include_directories(ut INTERFACE ${CMAKE_SOURCE_DIR}/vendor/)
    # MSVC is bogus with modules right now...
    target_compile_definitions(ut INTERFACE BOOST_UT_DISABLE_MODULE)
    target_compile_features(ut INTERFACE cxx_std_20)
endif()

# This will be local for each add_subdirectory and reverted as we exit it.
# We can use this behavior to check if a component inside a project was or
# was not yet created.
set(all_tests_target_created FALSE)

# add_our_test creates a new executable target.
# It's name will be the name provided in the argument plus '-test' suffix.
#
# Additionally, as soon as first test is defined, a test aggregating
# target is created, that will execute all of the project's tests
# using CTest.
function(add_our_test name)
    add_executable(${name}-test)
    target_link_libraries(${name}-test PRIVATE ut)
    add_test(NAME ${name} COMMAND ${name}-test)

    # Create a run-all target for tests, when creating the first test for the current project.
    # ctest should never compile the tests themselves, because we lose the progress feedback
    # from the IDEs and bulid systems. Instead we add CMake target dependencies to this
    # custom target instead.
    if(NOT ${all_tests_target_created})
        set(all_tests_target_created TRUE PARENT_SCOPE)
        add_custom_target(all-tests
            ctest --output-on-failure --test-dir ${CMAKE_BINARY_DIR} -C "$<CONFIG>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

    add_dependencies(all-tests ${name}-test)
endfunction()
