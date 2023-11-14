## `go-error`

`go-error` presents an alternative error API for C++ heavily based on the Go programming language error model.

### Features

* Errors with context
* Error wrapping
* Ability to create custom errors
* Predefined errors: `go::error_string`, `go::error_code`

### Roadmap

- [ ] Port hashicorp/multierror
- [ ] CMake subproject support
- [ ] Usage documentation and code documentation (postponed until API stabilizes)
- [ ] More platform/compiler compatibility tests
- [ ] Create multiple demo programs
- [ ] See if C++ standard can be lowered, if anybody needs that that is

### Compatibility

This library is only tested against VS2022 and VS2019 edition MSVC compilers right now

### Development

To build the project using CMake:
```
cmake -S . -B build
cmake --build build --target go-error
```

To run the tests make sure that CMake flag `BUILD_TESTING` is set to true (true by default) and run
```
cmake --build build --target all-tests
```

The examples are built as part of `all-tests` target, but not executed. Examples are standalone demo applications. You can prevent them from compiling by setting CMake flag `BUILD_EXAMPLES` to `OFF`.
