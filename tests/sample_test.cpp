#include <catch2/catch_test_macros.hpp>

TEST_CASE("sanity check: basic arithmetic", "[sanity]") {
    REQUIRE(1 + 1 == 2);
    REQUIRE(2 * 3 == 6);
}
