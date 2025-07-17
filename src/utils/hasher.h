#ifndef HASHER_H
#define HASHER_H
#include <cstdint>

namespace fw
{
   namespace hash
   {
      typedef std::uint32_t u32;
      constexpr u32 hash_buffer(const char* data, std::size_t len)
      {
	 u32 b = 378551;
	 u32 a = 63689;
	 u32 hashout = 0;
	 for(std::size_t i = 0; i < len; i++)
	 {
	    hashout = hashout * a + (u32)data[i];
	    a = a * b;
	 }
	 return (hashout & 0x7FFFFFFF);
      }
      template <typename T>
      constexpr u32 hash32(T&& s)
      {
	 return hash_buffer((const char*)&s, sizeof(T));
      }
      // todo: add 64 bit variant
      // origin: https://gist.github.com/Lee-R/3839813
      // FNV-1a 32bit hashing algorithm.
      constexpr u32 i32(char const* s, std::size_t count)
      {
	 // potentially large iteration time here.
	 return ((count ? i32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
      }
      // todo: try to make this always work.
      // todo: make alignment check compile time assert.
      // todo: why have I removed a single byte. that seems dumb.
      template <typename T>
      constexpr u32 i32(T&& s)
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
	 constexpr string(u32 hash) :
	    m_literal(nullptr),
	    m_len(0),
	    m_hash(hash)
	 {
	 }
	 constexpr string() = default;
	 constexpr operator u32() const { return m_hash; }
	 constexpr bool operator< (const string& rhs) const {return u32(*this) < u32(rhs); }
	 constexpr bool operator> (const string& rhs) const { return rhs < *this; }
	 constexpr bool operator<=(const string& rhs) const { return !(*this > rhs); }
	 constexpr bool operator>=(const string& rhs) const { return !(*this < rhs); }
	 
	 constexpr bool is_valid() const { return m_len != 0; }
	 
	 const char* m_literal = nullptr;
	 std::size_t m_len{0};
	 u32 m_hash{0};
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
	    for(u32 i = 0; i < _len; i++)
	    {
	       if(m_path[i] == path_separator)
	       {
		  starts[m_name_count++] = i+1;
	       }
	    }
	    starts[m_name_count] = _len;
	    for(u32 i = 0; i < m_name_count; i++)
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
	    for(u32 i = 0; i < _len; i++)
	    {
	       if(m_path[i] == path_separator)
	       {
		  starts[m_name_count++] = i+1;
	       }
	    }
	    starts[m_name_count] = _len;
	    for(u32 i = 0; i < m_name_count; i++)
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
	 u32 m_name_count = 1;
	 u32 m_hashes[m_max_names];
	 u32 m_hash;
	 //
	 u32 m_str_lens[m_max_names];
	 u32 m_path_len;
	 const char* m_names[m_max_names];
	 const char* m_path;
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

#endif//HASHER_H
