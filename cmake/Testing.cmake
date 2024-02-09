## Find GoogleTest
if(${GOERROR_BUILD_TESTING})
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

function(add_test_suite_dependency target)
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

    add_dependencies(all-tests ${target})
endfunction()

# add_our_test creates a new executable target.
# It's name will be the name provided in the argument plus '-test' suffix.
#
# Additionally, as soon as first test is defined, a test aggregating
# target is created, that will execute all of the project's tests
# using CTest.
function(add_our_test name)
    add_executable(test-${name})
    target_link_libraries(test-${name} PRIVATE ut)
    add_test(NAME ${name} COMMAND test-${name})

    # !!! Copypaste of add_test_suite_dependency
    # Because we can only get all_test_target_created var from
    # the parent context and in a recursive call the parent scopy
    # will be the scope of this function and not the global scope,
    # so the flag will net be able to be updated

    if(NOT ${all_tests_target_created})
        set(all_tests_target_created TRUE PARENT_SCOPE)
        add_custom_target(all-tests
            ctest --output-on-failure --test-dir ${CMAKE_BINARY_DIR} -C "$<CONFIG>"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
    endif()

    add_dependencies(all-tests test-${name})
endfunction()
