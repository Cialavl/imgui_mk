#pragma once
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

// mk::string<N>：定容量字符串（栈上缓冲区，无堆分配）。
// 不变量：m_Buf[m_Size] == '\0'（缓冲区永远以终止符结尾），m_Size == strlen。
// 溢出策略：内容长度超过 N-1 时 std::exit(EXIT_FAILURE)（与历史行为一致，
// death test 覆盖）。
namespace mk {
	namespace detail {
		// 限长 strlen：最多扫 maxLen 字节（未终止入参防越界读，可移植不依赖 POSIX strnlen）
		inline std::size_t clampedLen(const char* s, std::size_t maxLen) {
			std::size_t n = 0;
			while(n < maxLen && s[n] != '\0') ++n;
			return n;
		}
	}
	template<std::size_t N = 15>
	class string {
	public:
		string() {
			clear();
		}

		// 从 C 字符串构造：按真实内容长度判断（strnlen 截到 N 防止未终止入参越界读）
		string(const char* str) {
			if(!str) {
				clear();
				return;
			}
			assign(str, detail::clampedLen(str, N));
		}

		string(const string& str) {
			assign(str.m_Buf, str.m_Size);
		}

		string(string&& str) noexcept {
			assign(str.m_Buf, str.m_Size);
			str.clear();   // 源保持“空但有效”状态（可析构/可复用）
		}

		// 跨容量拷贝：容量不同只看内容长度（旧版按对方缓冲区容量判断，
		// string<32>("hi") 拷给 string<16> 会误杀进程）
		template<std::size_t M> requires (M != N)
		string(const string<M>& str) {
			assign(str.data(), str.size());
		}

		template<std::size_t M> requires (M != N)
		string(string<M>&& str) noexcept {
			assign(str.data(), str.size());
			str.clear();
		}

		string& operator=(const string& str) {
			if(this == &str) return *this;
			assign(str.m_Buf, str.m_Size);
			return *this;
		}

		string& operator=(string&& str) noexcept {
			if(this == &str) return *this;
			assign(str.m_Buf, str.m_Size);
			str.clear();
			return *this;
		}

		string& operator=(const char* str) {
			if(!str) {
				clear();
				return *this;
			}
			assign(str, detail::clampedLen(str, N));
			return *this;
		}

		~string() = default;

		// —— 访问 ——

		const char* c_str() const { return m_Buf; }
		const char* data() const { return m_Buf; }
		std::size_t size() const { return m_Size; }
		std::size_t length() const { return m_Size; }
		bool empty() const { return m_Size == 0; }
		constexpr std::size_t capacity() const { return N - 1; }

		char& operator[](std::size_t pos) { return m_Buf[pos]; }             // 调用方保证 pos <= size
		const char& operator[](std::size_t pos) const { return m_Buf[pos]; }

		// 可写缓冲区访问（ImGui InputText 等需要 char* 的接口）
		char* data() { return m_Buf; }

		void clear() {
			m_Size = 0;
			m_Buf[0] = '\0';
		}

		void assign(const char* str) {
			if(!str) {
				clear();
				return;
			}
			assign(str, detail::clampedLen(str, N));
		}

		std::string std_str() const {
			return std::string(m_Buf, m_Size);
		}

		std::string_view str_view() const {
			return std::string_view(m_Buf, m_Size);
		}

		friend std::ostream& operator<<(std::ostream& os, const string& str) {
			return os.write(str.m_Buf, (std::streamsize)str.m_Size);
		}

	private:
		// 统一赋值利器：len > N-1 直接终止（溢出策略，见文件头注释）；
		// 否则 memcpy + 维护不变量。注意 memcpy 语义——内容绝不被当格式串
		void assign(const char* str, std::size_t len) {
			if(len > N - 1) {
				std::exit(EXIT_FAILURE);
			}
			std::memcpy(m_Buf, str, len);
			m_Size = len;
			m_Buf[len] = '\0';
		}

		char m_Buf[N];
		std::size_t m_Size;
	};
}