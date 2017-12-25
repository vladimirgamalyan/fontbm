#include "../external/catch.hpp"
#include "extractFileName.h"

TEST_CASE("extractFileName")
{
    REQUIRE(extractFileName("foo/bar.zip") == "bar.zip");
    REQUIRE(extractFileName("foo/bar") == "bar");
    REQUIRE(extractFileName("foo\\bar") == "bar");
    REQUIRE(extractFileName("bar") == "bar");
    REQUIRE(extractFileName("c://foo/bar.gz") == "bar.gz");
    REQUIRE(extractFileName("c://foo/baz\\bar.gz") == "bar.gz");
    REQUIRE(extractFileName("foo\\\\//bar") == "bar");
    REQUIRE(extractFileName("../bar") == "bar");
}
