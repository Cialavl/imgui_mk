#include <gtest/gtest.h>
#include <string_mk/mk_string.h>

// mk::string<N> 固定容量字符串的基础行为
TEST(MkString, DefaultConstructor) {
	mk::string<16> s;
	EXPECT_STREQ(s.data(), "");
	EXPECT_EQ(s.std_str(), "");
}

TEST(MkString, ConstructorFromLiteral) {
	mk::string<8> s("Pw");
	EXPECT_EQ(s.std_str(), "Pw");
	EXPECT_STREQ(s.data(), "Pw");
	EXPECT_EQ(s.str_view(), "Pw");
}

TEST(MkString, CopyConstructor) {
	mk::string<16> a("Pw");
	mk::string<16> b(a);
	EXPECT_EQ(b.std_str(), "Pw");
}

TEST(MkString, AssignCharPtr) {
	mk::string<16> s;
	char buf[] = "hi";
	s = buf;
	EXPECT_EQ(s.std_str(), "hi");
	EXPECT_STREQ(s.data(), "hi");
}

TEST(MkString, AssignString) {
	mk::string<16> s;
	mk::string<16> a("xy");
	s = a;
	EXPECT_EQ(s.std_str(), "xy");
}

TEST(MkString, StdStrRoundTrip) {
	mk::string<32> s("hello");
	EXPECT_EQ(s.std_str(), std::string("hello"));
	EXPECT_EQ(s.str_view(), std::string_view("hello"));
}

// 容量不足时构造会 std::exit(EXIT_FAILURE)（death test 子进程验证）
TEST(MkStringDeathTest, OverflowCapacityExits) {
	EXPECT_DEATH({ mk::string<4> s("12345"); }, ".*");
}