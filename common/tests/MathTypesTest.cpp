#include <MathTypes.hpp>
#include "TestMain.hpp"

namespace spark {
using namespace common::math;
namespace tests {

TEST_F(UnitTests, MathTypes_Position) {
    {
        PositionF a = { 0, 0 };
        EXPECT_EQ(a.x(), 0.0f);
        EXPECT_EQ(a.y(), 0.0f);
        PositionF b = { 0, 1 };
        EXPECT_EQ(b.x(), 0.0f);
        EXPECT_EQ(b.y(), 1.0f);
        EXPECT_EQ(skDistance(a, b), 1.0f);
        EXPECT_EQ(skDistanceSq(a, b), 1.0f);
        a.x() = 5.0f;
        a.y() = 1.0f;
        EXPECT_EQ(a.x(), 5.0f);
        EXPECT_EQ(a.y(), 1.0f);
        EXPECT_EQ(skDistance(a, b), 5.0f);
        EXPECT_EQ(skDistanceSq(a, b), 25.0f);
    }
    {
        PositionF a = { 0, 0 };
        EXPECT_EQ(a.x(), 0.0f);
        EXPECT_EQ(a.y(), 0.0f);
        PositionF b = { 0, 2 };
        EXPECT_EQ(b.x(), 0.0f);
        EXPECT_EQ(b.y(), 2.0f);
        EXPECT_EQ(skDistance(a, b), 2.0f);
        EXPECT_EQ(skDistanceSq(a, b), 4.0f);
    }
}

TEST_F(UnitTests, MathTypes_Size) {
    {
        SizeF a = { 0, 0 };
        EXPECT_EQ(a.w(), 0.0f);
        EXPECT_EQ(a.h(), 0.0f);
        SizeF b = { 0, 1 };
        EXPECT_EQ(b.w(), 0.0f);
        EXPECT_EQ(b.h(), 1.0f);
        a.w() = 5.0f;
        a.h() = 1.0f;
        EXPECT_EQ(a.w(), 5.0f);
        EXPECT_EQ(a.h(), 1.0f);
    }
    {
        SizeF a = { 0, 0 };
        EXPECT_EQ(a.w(), 0.0f);
        EXPECT_EQ(a.h(), 0.0f);
        SizeF b = { 0, 2 };
        EXPECT_EQ(b.w(), 0.0f);
        EXPECT_EQ(b.h(), 2.0f);
    }
}

};
};
