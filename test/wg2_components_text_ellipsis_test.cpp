#include <gtest/gtest.h>

#include "wg2_test_harness.h"
#include <string>

// 第一层 · 文本省略与 UTF-8 码点边界

using namespace Mk::wg2;
using wg2test::Harness;
using wg2test::RectProbe;
using wg2test::isValidUtf8;

TEST(Wg2Component, EllipsizeKeepsUtf8Boundaries) {
	Harness h;
	h.inFrame([&] {
		const std::string text = "基础：wg2 直接 struct 控件的中文截断测试";
		EXPECT_EQ(detail::ellipsize(text, 100000.0f), text);   // 放得下原样返回
		EXPECT_EQ(detail::ellipsize(text, 0.0f), text);        // 非法宽度原样返回

		int truncated = 0;
		for (float avail : {240.0f, 160.0f, 90.0f, 40.0f, 12.0f}) {
			const std::string out = detail::ellipsize(text, avail);
			EXPECT_TRUE(isValidUtf8(out)) << "avail=" << avail;
			if (out == text) continue;   // 放得下 → 原样返回，不参与截断断言
			++truncated;
			ASSERT_GE(out.size(), 3u);
			EXPECT_EQ(out.substr(out.size() - 3), "...") << "avail=" << avail;
			// 去掉省略号后必须是原文的前缀（没跨码点、没丢字）
			const std::string head = out.substr(0, out.size() - 3);
			EXPECT_EQ(text.compare(0, head.size(), head), 0) << "avail=" << avail;
		}
		EXPECT_GE(truncated, 3);                       // 至少覆盖几档截断
		EXPECT_EQ(detail::ellipsize(text, 1.0f), std::string("..."));   // 极窄只剩省略号
	});
}

TEST(Wg2Component, Utf8CharLenClassifiesSequences) {
	EXPECT_EQ(detail::utf8CharLen(0x41), 1u);   // 'A'
	EXPECT_EQ(detail::utf8CharLen(0xC3), 2u);   // 2 字节首字节
	EXPECT_EQ(detail::utf8CharLen(0xE5), 3u);   // 3 字节首字节（中文）
	EXPECT_EQ(detail::utf8CharLen(0xF0), 4u);   // 4 字节首字节
	EXPECT_EQ(detail::utf8CharLen(0xFF), 1u);   // 非法首字节按 1 前进，不越界
}
