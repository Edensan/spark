#include <ValueTypes.hpp>
#include "TestMain.hpp"
#include <niLang/STL/limits.h>

namespace spark {
using namespace common;
using namespace common::math;
namespace tests {

TEST_F(UnitTests, ValueTypes) {
    u8 u8_ = astl::numeric_limits<u8>::max();
    u16 u16_ = astl::numeric_limits<u16>::max();
    u32 u32_ = astl::numeric_limits<u32>::max();
    u64 u64_ = astl::numeric_limits<u64>::max();
    i16 i16_ = astl::numeric_limits<i16>::min();
    i32 i32_ = astl::numeric_limits<i32>::min();
    i64 i64_ = astl::numeric_limits<i64>::min();
    f32 f32_ = astl::numeric_limits<f32>::max();
    f64 f64_ = astl::numeric_limits<f64>::max();
    Array2F arr2F_ = Array2F({ f32_, f32_ });
    astl::string s = "TestString";
    {
        Value v { u8_ };
        EXPECT_EQ(v.type(), Value::Type::U8);
        EXPECT_EQ(v.refAs<u8>(), u8_);
        EXPECT_EQ(v.constRefAs<u8>(), u8_);
        EXPECT_EQ(v.valueAs<u8>(), u8_);
    }
    {
        Value v { u16_ };
        EXPECT_EQ(v.type(), Value::Type::U16);
        EXPECT_EQ(v.refAs<u16>(), u16_);
        EXPECT_EQ(v.constRefAs<u16>(), u16_);
        EXPECT_EQ(v.valueAs<u16>(), u16_);
    }
    {
        Value v { u32_ };
        EXPECT_EQ(v.type(), Value::Type::U32);
        EXPECT_EQ(v.refAs<u32>(), u32_);
        EXPECT_EQ(v.constRefAs<u32>(), u32_);
        EXPECT_EQ(v.valueAs<u32>(), u32_);
    }
    {
        Value v { u64_ };
        EXPECT_EQ(v.type(), Value::Type::U64);
        EXPECT_EQ(v.refAs<u64>(), u64_);
        EXPECT_EQ(v.constRefAs<u64>(), u64_);
        EXPECT_EQ(v.valueAs<u64>(), u64_);
    }
    {
        Value v { i16_ };
        EXPECT_EQ(v.type(), Value::Type::I16);
        EXPECT_EQ(v.refAs<i16>(), i16_);
        EXPECT_EQ(v.constRefAs<i16>(), i16_);
        EXPECT_EQ(v.valueAs<i16>(), i16_);
    }
    {
        Value v { i32_ };
        EXPECT_EQ(v.type(), Value::Type::I32);
        EXPECT_EQ(v.refAs<i32>(), i32_);
        EXPECT_EQ(v.constRefAs<i32>(), i32_);
        EXPECT_EQ(v.valueAs<i32>(), i32_);
    }
    {
        Value v { i64_ };
        EXPECT_EQ(v.type(), Value::Type::I64);
        EXPECT_EQ(v.refAs<i64>(), i64_);
        EXPECT_EQ(v.constRefAs<i64>(), i64_);
        EXPECT_EQ(v.valueAs<i64>(), i64_);
    }
    {
        Value v { f32_ };
        EXPECT_EQ(v.type(), Value::Type::F32);
        EXPECT_EQ(v.refAs<f32>(), f32_);
        EXPECT_EQ(v.constRefAs<f32>(), f32_);
        EXPECT_EQ(v.valueAs<f32>(), f32_);
    }
    {
        Value v { f64_ };
        EXPECT_EQ(v.type(), Value::Type::F64);
        EXPECT_EQ(v.refAs<f64>(), f64_);
        EXPECT_EQ(v.constRefAs<f64>(), f64_);
        EXPECT_EQ(v.valueAs<f64>(), f64_);
    }
    {
        Value v { arr2F_ };
        EXPECT_EQ(v.type(), Value::Type::Array2F);
        EXPECT_TRUE(v.refAs<Array2F>() == arr2F_);
        EXPECT_TRUE(v.constRefAs<Array2F>() == arr2F_);
        EXPECT_TRUE(v.valueAs<Array2F>() == arr2F_);
    }
    {
        Value v1 { s, false };
        EXPECT_EQ(v1.type(), Value::Type::String);
        EXPECT_EQ(*v1.refAs<astl::string *>(), s);
        EXPECT_EQ(*v1.constRefAs<astl::string *>(), s);
        EXPECT_EQ(*v1.valueAs<astl::string *>(), s);

        Value v2 { s, true };
        EXPECT_EQ(v2.type(), Value::Type::AllocatedString);
        EXPECT_EQ(*v2.refAs<astl::string *>(), s);
        EXPECT_EQ(*v2.constRefAs<astl::string *>(), s);
        EXPECT_EQ(*v2.valueAs<astl::string *>(), s);

        EXPECT_EQ(v1.refAs<astl::string *>(), &s);
        EXPECT_NE(v2.refAs<astl::string *>(), &s);
    }
}

};
};
