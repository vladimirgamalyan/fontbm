#include "catch.hpp"
#include "../Config.h"
#include "../ProgramOptions.h"

TEST_CASE( "parseColor")
{
    REQUIRE((helpers::parseColor("0,0,0") == Config::Color{0, 0, 0}));
    REQUIRE((helpers::parseColor("255,255,255") == Config::Color{255, 255, 255}));
    REQUIRE((helpers::parseColor(" 255 , 255    ,  255  ") == Config::Color{255, 255, 255}));

    REQUIRE_THROWS_AS(helpers::parseColor(""), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseColor("foo"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseColor("0,1"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseColor("0,1,2,3"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseColor("0,a,1"), std::logic_error);

    REQUIRE_THROWS_AS(helpers::parseColor("0,1,256"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseColor("0,1,-1"), std::logic_error);
}
