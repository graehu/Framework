#ifndef HASHER_H
#define HASHER_H
#include <cstdint>
namespace fw
{
   namespace hash
   {
      // origin: https://gist.github.com/Lee-R/3839813
      // FNV-1a 32bit hashing algorithm.
      constexpr std::uint32_t i32(char const* s, std::size_t count)
      {
	 // potentially large iteration time here.
	 return ((count ? i32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
      }
      template <typename T>
      constexpr std::uint32_t i32(T&& s)
      {
	 return i32(s, sizeof(T)-1);
      }
      //
      struct string
      {
	 template<std::size_t N>
	 constexpr string(const char(&_literal)[N]) :
	    m_literal(&_literal[0]),
	    m_len(N-1),
	    m_hash(i32(m_literal, m_len-1))
	 {
	 }
	 constexpr string(const char* _literal, std::size_t _size) :
	    m_literal(_literal),
	    m_len(_size),
	    m_hash(i32(m_literal, m_len-1))
	 {
	 }
	 constexpr string() = default;
	 const char* m_literal = nullptr;
	 const std::size_t m_len{0};
	 const std::uint32_t m_hash{0};
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
	    m_hash = i32(m_path, _len-1);
	    int starts[path::m_max_names] = {0};
	    starts[0] = 0;
	    for(std::uint32_t i = 0; i < _len; i++)
	    {
	       if(m_path[i] == path_separator)
	       {
		  starts[m_name_count++] = i+1;
	       }
	    }
	    starts[m_name_count] = _len;
	    for(std::uint32_t i = 0; i < m_name_count; i++)
	    {
	       auto str_len = ((starts[i+1]-1)-starts[i]);
	       if(i == m_name_count-1)
	       {
		  str_len+=1;
	       }
	       m_str_lens[i] = str_len;
	       m_hashes[i] = i32(&m_path[starts[i]], str_len-1);
	       m_names[i] = &m_path[starts[i]];
	    }
	    m_names[m_name_count] = &m_path[_len+1];
	 }
	 path(const char* _path, std::size_t _len)
	 {
	    m_path = _path;
	    m_path_len = _len;
	    m_hash = i32(m_path, _len-1);
	    int starts[path::m_max_names] = {0};
	    starts[0] = 0;
	    for(std::uint32_t i = 0; i < _len; i++)
	    {
	       if(m_path[i] == path_separator)
	       {
		  starts[m_name_count++] = i+1;
	       }
	    }
	    starts[m_name_count] = _len;
	    for(std::uint32_t i = 0; i < m_name_count; i++)
	    {
	       auto str_len = ((starts[i+1]-1)-starts[i]);
	       if(i == m_name_count-1)
	       {
		  str_len+=1;
	       }
	       m_str_lens[i] = str_len;
	       m_hashes[i] = i32(&m_path[starts[i]], str_len-1);
	       m_names[i] = &m_path[starts[i]];
	    }
	    m_names[m_name_count] = &m_path[_len+1];
	 }

	 static const int m_max_names = 8;
	 std::uint32_t m_name_count = 1;
	 std::uint32_t m_hashes[m_max_names];
	 std::uint32_t m_hash;
	 //
	 std::uint32_t m_str_lens[m_max_names];
	 std::uint32_t m_path_len;
	 const char* m_names[m_max_names];
	 const char* m_path;
      };
   }
}
#endif//HASHER_H
