#include "hello_support.hpp"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("hello string contains expected word", "[hello]") {
    const auto message = hello::message_for("catch2 test");
    REQUIRE(hello::contains_hello(message));
}
