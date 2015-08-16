#include "testing.h"
#include "../CharList.h"
#include <stdexcept>

BEGIN_TEST()
        EXPECT_EQUAL(charListToString(CharList{CharListItem{0, 1}}), "0-1");
        EXPECT_EQUAL(charListToString(CharList{CharListItem{10, 20}}), "10-20");
        EXPECT_EQUAL(charListToString(CharList{CharListItem{10, 10}}), "10");
        EXPECT_EQUAL(charListToString(CharList{CharListItem{10, 20}, CharListItem{30, 30}}), "10-20,30");
        EXPECT_EQUAL(charListToString(CharList{CharListItem{10, 20}, CharListItem{30, 40}}), "10-20,30-40");
        EXPECT_EQUAL(charListToString(CharList{CharListItem{10, 20}, CharListItem{30, 40}, CharListItem{50, 60}}), "10-20,30-40,50-60");

        EXPECT_EXCEPTION(parseCharListString("0a"), std::logic_error);
        EXPECT_EXCEPTION(parseCharListString(","), std::logic_error);
        EXPECT_EXCEPTION(parseCharListString("-"), std::logic_error);
        EXPECT_EXCEPTION(parseCharListString("10,20,"), std::logic_error);

        EXPECT_EQUAL(parseCharListString("0"), CharList{CharListItem{0, 0}});
        EXPECT_EQUAL(parseCharListString("10,20"), CharList{CharListItem{10, 10}, CharListItem{20, 20}});
        EXPECT_EQUAL(parseCharListString("10, 20"), CharList{CharListItem{10, 10}, CharListItem{20, 20}});
        EXPECT_EQUAL(parseCharListString("10-20,30-40"), CharList{CharListItem{10, 20}, CharListItem{30, 40}});
        EXPECT_EQUAL(parseCharListString("10-20,15,30-40"), CharList{CharListItem{10, 20}, CharListItem{15, 15}, CharListItem{30, 40}});

        EXPECT_EXCEPTION(parseCharListString("65535-65536"), std::logic_error);
        EXPECT_EXCEPTION(parseCharListString("1--2"), std::logic_error);
        EXPECT_EXCEPTION(parseCharListString("-1-2"), std::logic_error);
END_TEST()
