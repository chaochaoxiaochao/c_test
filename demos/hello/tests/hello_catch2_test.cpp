#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <string>

TEST_CASE("hello string contains expected word", "[hello]") {
    const std::string message = "hello catch2 test";
    REQUIRE(message.find("hello") != std::string::npos);
}
