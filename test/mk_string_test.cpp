#include <gtest/gtest.h>
#include <string_mk/mk_string.h>
#include <cstring>
#include <utility>

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

// —— 修复回归与新增接口 ——

// 旧版按对方缓冲区容量判断：string<32>("hi") 拷给 string<16> 会误 exit；
// 现按内容长度判断，跨容量小内容拷贝/移动应正常
TEST(MkString, CrossCapacityCopy) {
	mk::string<32> a("hi");
	mk::string<16> b(a);
	EXPECT_EQ(b.std_str(), "hi");

	mk::string<32> c("xy");
	mk::string<16> d(std::move(c));
	EXPECT_EQ(d.std_str(), "xy");
	EXPECT_TRUE(c.empty());   // 移动源保持空但有效
}

// 旧版移动只搬 string_view（悬垂）且不拷字符：移动后目标应持有内容副本
TEST(MkString, MoveCopiesCharacters) {
	mk::string<16> a("payload");
	mk::string<16> b(std::move(a));
	EXPECT_EQ(b.std_str(), "payload");
	EXPECT_EQ(a.std_str(), "");   // 源被清空（而非悬垂）
	EXPECT_STREQ(a.data(), "");   // 源仍可安全访问
}

// 旧版 snprintf 把内容当格式串：含 % 的内容是 UB，现 memcpy 语义安全
TEST(MkString, PercentContentIsSafe) {
	mk::string<32> s("%s%d%n%%");
	EXPECT_EQ(s.std_str(), "%s%d%n%%");
	mk::string<32> t;
	t = "%s";
	EXPECT_EQ(t.std_str(), "%s");
}

// 旧版拷贝构造收非 const 引用：const 对象/临时对象无法拷贝
TEST(MkString, CopyFromConst) {
	const mk::string<16> a("c");
	mk::string<16> b(a);
	EXPECT_EQ(b.std_str(), "c");
	b = a;   // const 赋值
	EXPECT_EQ(b.std_str(), "c");
}

// 真实长度维护（旧版 size 字段是 sizeof(指针) 垃圾值）
TEST(MkString, SizeIsStringLength) {
	mk::string<16> s("hello");
	EXPECT_EQ(s.size(), 5u);
	EXPECT_EQ(s.length(), 5u);
	EXPECT_FALSE(s.empty());
	s.clear();
	EXPECT_EQ(s.size(), 0u);
	EXPECT_TRUE(s.empty());
}

// 字符访问与可写缓冲区
TEST(MkString, ElementAccess) {
	mk::string<16> s("abc");
	EXPECT_EQ(s[0], 'a');
	EXPECT_EQ(s[2], 'c');
	s[0] = 'X';
	EXPECT_EQ(s.std_str(), "Xbc");

	// 可写 data()（ImGui InputText 场景）+ assign 重设
	std::memcpy(s.data(), "new", 3);
	s.assign("replaced");
	EXPECT_EQ(s.std_str(), "replaced");
}

// 自赋值安全（旧版移动/拷贝自赋值路径未正确维护不变量）
TEST(MkString, SelfAssignment) {
	mk::string<16> s("self");
	s = s;
	EXPECT_EQ(s.std_str(), "self");
	s = std::move(s);
	EXPECT_EQ(s.std_str(), "self");
}

// 精确容量边界：恰好 N-1 字符可存，N 字符 exit
TEST(MkString, ExactCapacityBoundary) {
	mk::string<5> s("abcd");   // 4 == N-1
	EXPECT_EQ(s.size(), 4u);
	EXPECT_EQ(s.std_str(), "abcd");
}

TEST(MkStringDeathTest, BoundaryOverflowExits) {
	EXPECT_DEATH({ mk::string<5> s("abcde"); }, ".*");   // 5 == N
}

// 空指针容忍：构造/赋值 nullptr 得空串而非崩溃
TEST(MkString, NullptrTolerant) {
	mk::string<8> a(nullptr);
	EXPECT_TRUE(a.empty());
	mk::string<8> b;
	b = nullptr;
	EXPECT_TRUE(b.empty());
}