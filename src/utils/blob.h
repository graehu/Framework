#pragma once
#include <cstddef>
#include <stdio.h>

namespace blob
{
   template<typename T> struct Buffer {const T* data = nullptr; size_t len = 0;};
   void init();
   char* allocate(size_t);
   void shutdown();
   template<typename T>
   inline bool save(const char* in_filename, T in_buffer)
   {
      FILE* file = fopen(in_filename, "wb");
      if (file == nullptr) { return false; }
      fwrite(in_buffer.data, sizeof(*in_buffer.data), in_buffer.len, file);
      fclose(file);
      return true;
   }
   template<typename T>
   inline bool load(const char* in_filename, T& out_buffer)
   {
      FILE* file = fopen(in_filename, "rb");
      if (file == nullptr) { return false; }
      fseek(file, 0, SEEK_END);
      out_buffer.len = ftell(file);
      fseek(file, 0, SEEK_SET);
      out_buffer.data = (decltype(out_buffer.data))allocate(out_buffer.len + 1);
      /* out_buffer.data = (decltype(out_buffer.data)) new char[out_buffer.len + 1]; */
      fread((char*)out_buffer.data, out_buffer.len, 1, file);
      fclose(file);
      out_buffer.len = out_buffer.len / sizeof(*out_buffer.data);
      return true;
   }
}

