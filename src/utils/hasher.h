#pragma once
#include <cstdint>

namespace fw
{
	namespace hash
	{
		typedef std::uint32_t u32;
		constexpr u32 hash_buffer(const char* data, const std::size_t len)
		{
			u32 b = 378551;
			u32 a = 63689;
			u32 hashout = 0;
			for (std::size_t i = 0; i < len; i++)
			{
				hashout = hashout * a + (u32)data[i];
				a = a * b;
			}
			return (hashout & 0x7FFFFFFF);
		}

		template<std::size_t N>
		constexpr u32 hash32(const char(&s)[N])
		{
			// note: N-1 to stop us hashing the null-terminator.
			// ----: not all strings are null terminated.
			// ----: hasing to the last valid character is more consistent / stable.
			return hash_buffer(s, N-1);
		}
		template <typename T>
		constexpr u32 hash32(T&& s)
		{
			return hash_buffer((const char*)&s, sizeof(T));
		}	
		struct string
		{
			template<std::size_t N>
			constexpr string(const char(&_literal)[N]) :
				m_literal(&_literal[0]),
				m_len(N-1),
				m_hash(hash_buffer(m_literal, N-1))
			{
			}
			constexpr string(const char* _literal, const std::size_t _size) :
				m_literal(_literal),
				m_len(_size),
				m_hash(hash_buffer(m_literal, _size))
			{
			}
			constexpr string(u32 hash) :
				m_literal(nullptr),
				m_len(0),
				m_hash(hash)
			{
			}
			constexpr string() = default;
			constexpr operator u32() const { return m_hash; }
			constexpr bool operator< (const string& rhs) const { return u32(*this) < u32(rhs); }
			constexpr bool operator> (const string& rhs) const { return rhs < *this; }
			constexpr bool operator<=(const string& rhs) const { return !(*this > rhs); }
			constexpr bool operator>=(const string& rhs) const { return !(*this < rhs); }

			constexpr bool is_valid() const { return m_len != 0; }

			const char* m_literal = nullptr;
			std::size_t m_len{ 0 };
			u32 m_hash{ 0 };
		};
		struct path
		{
			static const char path_separator = '.';
			template<std::size_t N>
			constexpr path(const char(&_path)[N]) :
				m_hashes(),
				m_hash(0),
				m_str_lens(),
				m_path_len(),
				m_names(),
				m_path(nullptr)
			{
				auto _len = N-1;
				m_path = _path;
				m_path_len = _len;
				m_hash = hash_buffer(m_path, _len);
				int starts[path::max_names] = { 0 };
				starts[0] = 0;
				for (u32 i = 0; i < _len; i++)
				{
					if (m_path[i] == path_separator)
					{
						starts[m_name_count++] = i + 1;
					}
				}
				starts[m_name_count] = _len;
				for (u32 i = 0; i < m_name_count; i++)
				{
					auto str_len = ((starts[i + 1] - 1) - starts[i]);
					if (i == m_name_count - 1)
					{
						str_len += 1;
					}
					m_str_lens[i] = str_len;
					m_hashes[i] = hash_buffer(&m_path[starts[i]], str_len);
					m_names[i] = &m_path[starts[i]];
				}
			}
			constexpr path(const char* _path, const std::size_t _len)
			{
				m_path = _path;
				m_path_len = _len;
				m_hash = hash_buffer(m_path, _len);
				int starts[path::max_names] = { 0 };
				starts[0] = 0;
				for (u32 i = 0; i < _len; i++)
				{
					if (m_path[i] == path_separator)
					{
						starts[m_name_count++] = i + 1;
					}
				}
				starts[m_name_count] = _len;
				for (u32 i = 0; i < m_name_count; i++)
				{
					auto str_len = ((starts[i + 1] - 1) - starts[i]);
					if (i == m_name_count - 1)
					{
						str_len += 1;
					}
					m_str_lens[i] = str_len;
					m_hashes[i] = hash_buffer(&m_path[starts[i]], str_len);
					m_names[i] = &m_path[starts[i]];
				}
			}

			static const int max_names = 8;
			u32 m_name_count = {1};
			u32 m_hashes[max_names] = {};
			u32 m_hash = {0};
			u32 m_str_lens[max_names] = {};
			u32 m_path_len = {0};
			const char* m_names[max_names] = {};
			const char* m_path = {nullptr};
		};
	}
} // namespace fw

// if i ever need = operators for whatever reason
// string& operator=(string& rhs)
// {
//    m_literal = rhs.m_literal;
//    m_len = rhs.m_len;
//    m_hash = rhs.m_hash;
//    return *this;
// }
// string& operator=(const string& rhs)
// {
//    m_literal = rhs.m_literal;
//    m_len = rhs.m_len;
//    m_hash = rhs.m_hash;
//    return *this;
// }
// string& operator=(string&& rhs)
// {
//    m_literal = std::move(rhs.m_literal);
//    m_len = std::move(rhs.m_len);
//    m_hash = std::move(rhs.m_hash);
//    return *this;
// }
// string& operator=(const string&& rhs)
// {
//    m_literal = std::move(rhs.m_literal);
//    m_len = std::move(rhs.m_len);
//    m_hash = std::move(rhs.m_hash);
//    return *this;
// }
// constexpr string(string& rhs) {*this = rhs;}
// constexpr string(string&& rhs) {*this = rhs;}
// constexpr string(const string& rhs) {*this = rhs;}
// constexpr string(const string&& rhs) {*this = rhs;}

