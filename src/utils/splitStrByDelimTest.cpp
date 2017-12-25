#include "../external/catch.hpp"
#include "splitStrByDelim.h"

TEST_CASE("splitStrByDelim")
{
    {
        std::vector<std::string> r = splitStrByDelim("1-2", '-');
        REQUIRE(r.size() == 2);
        REQUIRE(r[0] == "1");
        REQUIRE(r[1] == "2");
    }

    {
        std::vector<std::string> r = splitStrByDelim("1", '-');
        REQUIRE(r.size() == 1);
        REQUIRE(r[0] == "1");
    }

    {
        std::vector<std::string> r = splitStrByDelim("a/b/z", '/');
        REQUIRE(r.size() == 3);
        REQUIRE(r[0] == "a");
        REQUIRE(r[1] == "b");
        REQUIRE(r[2] == "z");
    }

    {
        std::vector<std::string> r = splitStrByDelim("--1---2-", '-');
        REQUIRE(r.size() == 7);
        REQUIRE(r[0] == "");
        REQUIRE(r[1] == "");
        REQUIRE(r[2] == "1");
        REQUIRE(r[3] == "");
        REQUIRE(r[4] == "");
        REQUIRE(r[5] == "2");
        REQUIRE(r[6] == "");
    }
}
