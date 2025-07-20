#pragma once
#include <cstddef>
#include <stdio.h>
#include "hasher.h"

#define GiBs << 30
#define MiBs << 20
#define KiBs << 10

// blob: binary serializable objects.
// bank: storage, loading, and serialisation structure.
namespace fw
{    
   namespace blob
   {
      // todo: add unload function.
      // todo: add content hash+tracking to buffers so we can read the hash and opt out of load.
      // todo: add ref count to allocations for duplicate loads.
      const hash::u32 fourcc = 'fwb1'; // 8 byte header.
      struct header {hash::u32 ver = fourcc; hash::u32 hash = 0; };
      template<typename T> struct Buffer {const T* data = nullptr; size_t len = 0; header head = {}; };
      template<typename T> struct BufferNc {T* data = nullptr; size_t len = 0; header head = {}; };
      typedef Buffer<char> Allocation;
      struct AllocNode
      {
	 Allocation alloc;
	 AllocNode* next = nullptr;
      };
      class bank final
      {
	public:
	 void init(size_t in_capacity, size_t in_page);
	 void shutdown();
      
	 template<typename T> inline bool save(const char* in_filename, T in_buffer)
	 {
	    FILE* file = fopen(in_filename, "wb");
	    if (file == nullptr) { return false; }
	    assert(in_buffer.head.ver == fourcc);
	    if(in_buffer.head.hash == 0)
	    {
	       in_buffer.head.hash = hash::hash_buffer((const char*)in_buffer.data, sizeof(*in_buffer.data)*in_buffer.len);
	    }
	    fwrite(&in_buffer.head, sizeof(in_buffer.head), 1, file);
	    fwrite(in_buffer.data, sizeof(*in_buffer.data), in_buffer.len, file);
	    fclose(file);
	    return true;
	 }
	 template<typename T> inline bool load(const char* in_filename, T& out_buffer)
	 {
	    FILE* file = fopen(in_filename, "rb");
	    if (file == nullptr) { return false; }
	    fseek(file, 0, SEEK_END);
	    out_buffer.len = ftell(file);
	    assert(((out_buffer.len-sizeof(out_buffer.head)) % sizeof(*out_buffer.data)) == 0);
	    fseek(file, 0, SEEK_SET);
	    out_buffer.data = (decltype(out_buffer.data))allocate(out_buffer.len + 1);
	    // todo: consider a short read
	    fread((char*)out_buffer.data, out_buffer.len, 1, file);
	    out_buffer.head = *((header*)out_buffer.data);
	    out_buffer.data = (decltype(out_buffer.data)) ((char*)out_buffer.data+sizeof(out_buffer.head));
	    assert(out_buffer.head.ver == fourcc);
	    // todo: decide if a file with no content is ok: (out_buffer.len == sizeof(out_buffer.head)).
	    assert(out_buffer.head.hash != 0 || out_buffer.len == sizeof(out_buffer.head));
	    fclose(file);
	    out_buffer.len = (out_buffer.len-sizeof(out_buffer.head)) / sizeof(*out_buffer.data);
	    return true;
	 }
	 template<typename T> bool free(Buffer<T>& in)
	 {
	    if(free((char*)in.data)-sizeof(in.head)) {in = {}; return true;}
	    return false;
	 }
	private:
	 char* allocate(size_t);
	 bool free(char*);
	 size_t capacity = 0;
	 size_t page = 0;
	 AllocNode* allocations = nullptr;
	 char* heap = nullptr;
	 char* end = nullptr;
	 AllocNode* freed = nullptr;
	 AllocNode* used = nullptr;
	 size_t total_allocations = 0;
      };
      extern bank miscbank;
   }
}
