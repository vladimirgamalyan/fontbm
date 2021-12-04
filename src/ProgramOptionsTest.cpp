#include "external/catch.hpp"
#include "ProgramOptions.h"

class Args
{
public:
    explicit Args(std::vector<std::string> args) : arg0("foo"), arguments(std::move(args))
    {
        pointers.push_back(const_cast<char*>(arg0.data()));
        for (auto& s : arguments)
            pointers.push_back(const_cast<char*>(s.data()));
    }

    int argc() const
    {
        return static_cast<int>(pointers.size());
    }

    char** argv()
    {
        return &pointers[0];
    }

private:
    std::string arg0;
    std::vector<std::string> arguments;
    std::vector<char*> pointers;
};

TEST_CASE( "parseCmdLine")
{
    {
        Args args({"--font-file", "vera.ttf", "--output", "vera"});
        Config config = ProgramOptions::parseCommandLine(args.argc(), args.argv());
        REQUIRE(config.fontFile == "vera.ttf");
        REQUIRE(config.output == "vera");
    }

    {
        Args args({"--font-file", "vera.ttf"});
        REQUIRE_THROWS_AS(ProgramOptions::parseCommandLine(args.argc(), args.argv()), std::runtime_error);
    }
}

TEST_CASE("parseColor")
{
    REQUIRE((ProgramOptions::parseColor("0,0,0") == Config::Color{0, 0, 0}));
    REQUIRE((ProgramOptions::parseColor("255,255,255") == Config::Color{255, 255, 255}));
    REQUIRE((ProgramOptions::parseColor(" 255 , 255    ,  255  ") == Config::Color{255, 255, 255}));

    REQUIRE_THROWS_AS(ProgramOptions::parseColor(""), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("foo"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("0,1"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("0,1,2,3"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("0,a,1"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("0,1,256"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseColor("0,1,-1"), std::logic_error);
}

TEST_CASE("parseCharsString")
{
    REQUIRE((ProgramOptions::parseCharsString("").empty()));
    REQUIRE((ProgramOptions::parseCharsString("0") == std::set<std::uint32_t>{0}));
    REQUIRE((ProgramOptions::parseCharsString("42") == std::set<std::uint32_t>{42}));
    REQUIRE((ProgramOptions::parseCharsString("0-1") == std::set<std::uint32_t>{0,1}));
    REQUIRE((ProgramOptions::parseCharsString("1-3") == std::set<std::uint32_t>{1,2,3}));
    REQUIRE((ProgramOptions::parseCharsString(" 1 - 3 ") == std::set<std::uint32_t>{1,2,3}));

    REQUIRE_THROWS_AS(ProgramOptions::parseCharsString("foo"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseCharsString("-1"), std::logic_error);
    REQUIRE_THROWS_AS(ProgramOptions::parseCharsString("-1-2"), std::logic_error);
}
