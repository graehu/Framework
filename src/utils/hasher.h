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
   constexpr std::uint32_t i32(T s)
   {
      // potentially large iteration time here.
      return ((sizeof(T) ? i32(s, sizeof(T) - 1) : 2166136261u) ^ s[sizeof(T)]) * 16777619u;
   }
}
#endif//HASHER_H
