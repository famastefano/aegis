# Testing

## C++ Unit Tests

GoogleTest/GoogleMock v1.17 is the approved C++ unit-test framework for this
repository. It is vendored under `thirdparty/googletest` so normal configure,
build, and test workflows do not need network access.

Use GoogleTest for new C++ unit tests and GoogleMock when interface mocking is
useful. Test targets should register with CTest.

Production binaries and libraries must not depend on GoogleTest or GoogleMock.
Only test targets should link against the `gtest`, `gtest_main`, `gmock`, or
`gmock_main` targets.

Prefer deterministic unit tests that do not require administrator privileges,
network access, backend services, live OS telemetry sessions, or machine-local
state. Integration tests that require those conditions should be separate from
the normal unit-test targets.

When testing project code, link the smallest practical library under test
rather than an executable entry point. Executables should stay thin enough that
most behavior is testable through libraries.
