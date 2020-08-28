#ifndef HASHER_H
#define HASHER_H
#include <cstdint>

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
   struct path
   {
      static const int m_max_names = 8;
      static const int m_max_len = 128;
      int m_name_count = 1;
      char* m_names[m_max_names];
      std::uint32_t m_hashes[m_max_names];
      char m_path[m_max_len];
      std::uint32_t m_hash;
   };
   constexpr path make_path(const char* _path, std::uint32_t _len, char _separator = '.')
   {
      path out = {};
      for(int i = 0; i < _len; i++)
      {
	 out.m_path[i] = _path[i];
      }
      out.m_path[_len] = '\0';
      out.m_hash = i32(out.m_path, _len);
      int starts[path::m_max_names] = {0};
      starts[0] = 0;
      for(int i = 0; i < _len; i++)
      {
	 if(out.m_path[i] == _separator)
	 {
	    starts[out.m_name_count++] = i+1;
	    out.m_path[i] = '\0';
	 }
      }
      starts[out.m_name_count] = _len;
      for(int i = 0; i < out.m_name_count; i++)
      {
	 auto str_len = starts[i+1]-starts[i];
	 out.m_hashes[i] = i32(&out.m_path[starts[i]], str_len-1);
	 out.m_names[i] = &out.m_path[starts[i]];
      }
      return out;
   }
   template <typename T>
   constexpr path make_path(T&& _path, char _separator = '.')
   {
      auto len = sizeof(T)-1;
      return make_path(_path, len, _separator);
   }
}
#endif//HASHER_H
