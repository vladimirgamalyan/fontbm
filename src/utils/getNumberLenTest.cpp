#include "../external/catch.hpp"
#include "getNumberLen.h"

TEST_CASE("getNumberLen")
{
    REQUIRE(getNumberLen(0) == 1);
    REQUIRE(getNumberLen(1) == 1);
    REQUIRE(getNumberLen(9) == 1);
    REQUIRE(getNumberLen(10) == 2);
    REQUIRE(getNumberLen(50) == 2);
    REQUIRE(getNumberLen(77) == 2);
    REQUIRE(getNumberLen(99) == 2);
    REQUIRE(getNumberLen(100) == 3);
    REQUIRE(getNumberLen(999) == 3);
    REQUIRE(getNumberLen(1000) == 4);
    REQUIRE(getNumberLen(10000) == 5);
}
