#include "catch.hpp"
#include "../Config.h"
#include "../ProgramOptions.h"
#include "../splitStrByDelim.h"
#include <iostream>

class Args
{
public:
    explicit Args(const std::vector<std::string>& args) : arg0("foo"), arguments(args)
    {
        pointers.push_back(arg0.data());
        for (auto& s : arguments)
            pointers.push_back(s.data());
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
        Config config = helpers::parseCommandLine(args.argc(), args.argv());
        REQUIRE(config.fontFile == "vera.ttf");
        REQUIRE(config.output == "vera");
        REQUIRE(config.textureSize.w == 256);
        REQUIRE(config.textureSize.h == 256);
    }

    {
        Args args({"--font-file", "vera.ttf"});
        REQUIRE_THROWS_AS(helpers::parseCommandLine(args.argc(), args.argv()), std::runtime_error);
    }
}

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

TEST_CASE("parseColor")
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

TEST_CASE("parseCharsString")
{
    REQUIRE((helpers::parseCharsString("") == std::set<uint32_t>{}));
    REQUIRE((helpers::parseCharsString("0") == std::set<uint32_t>{0}));
    REQUIRE((helpers::parseCharsString("42") == std::set<uint32_t>{42}));
    REQUIRE((helpers::parseCharsString("0-1") == std::set<uint32_t>{0,1}));
    REQUIRE((helpers::parseCharsString("1-3") == std::set<uint32_t>{1,2,3}));
    REQUIRE((helpers::parseCharsString(" 1 - 3 ") == std::set<uint32_t>{1,2,3}));

    REQUIRE_THROWS_AS(helpers::parseCharsString("foo"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseCharsString("-1"), std::logic_error);
    REQUIRE_THROWS_AS(helpers::parseCharsString("-1-2"), std::logic_error);
    //REQUIRE_THROWS_AS(helpers::parseCharsString("1-2 3"), std::logic_error);
}
