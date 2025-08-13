#pragma once
#include <cassert>
#include <cstddef>
#include <stdio.h>
#include "hasher.h"

#define GiBs << 30
#define MiBs << 20
#define KiBs << 10

// blob: binary serializable objects.
// bank: storage, loading, and serialisation structure.
// asset: loadable / serialisable object.
// note: when moving from "Allocation" to other buffers, len is the count of sizeof(T).
namespace fw
{    
   namespace blob
   {
      // todo: add unload function.
      // todo: add content hash+tracking to buffers so we can read the hash and opt out of load.
      // todo: add ref count to allocations for duplicate loads.
      // todo: page needs to control allocation size allocat(size) {((int)size/page)*page}
      const hash::u32 fourcc = 'fwb1'; // change to bnk1? header<T> ver = T::fourcc;
      struct header {hash::u32 ver = fourcc; hash::u32 hash = 0; }; // 8 byte header.
      // something like this?
      // template<typename T, size_T V=fourcc> struct asset {header head = {}; const T* data = nullptr; size_t len = 0; };
      template<typename T> struct asset {header head = {}; const T* data = nullptr; size_t len = 0; };
      template<typename T> struct assetnc {header head = {}; T* data = nullptr; size_t len = 0; };
      typedef asset<char> allocation;
      struct allocnode
      {
	 allocation alloc;
	 allocnode* next = nullptr;
      };
      class bank final
      {
	public:
	 void init(size_t in_capacity, size_t in_page);
	 void shutdown();
	 void print();
	 inline bool is_initialised() const { return heap != nullptr; }
	 inline size_t get_capacity() const { return  capacity; }
	 inline size_t get_used() const { return (size_t)(end-heap); }
	 inline int get_freecount() const {return freecount; }
	 inline int get_usedcount() const {return usedcount; }
	 inline allocnode* get_freednode() const { return freed; }
	 inline allocnode* get_usednode() const { return used; }
	 template<typename T> inline bool save(const char* in_filename, T in_buffer);
	 template<typename T> inline bool load(const char* in_filename, T& out_buffer);
	 template<typename T> inline bool free(asset<T>& in);
	 // template<typename T> inline bool allocate(asset<T>& out_buffer);
	 template<typename T> inline bool find(hash::u32 in_hash, asset<T>& out_buffer);
	 template<typename T> inline bool fixup(asset<T>& out_buffer);

	 allocation* allocate(size_t);
	 bool free(char*);
	 
	private:
	 // allocation* allocate(size_t);
	 // bool free(char*);
	 int freecount = 0;
	 int usedcount = 0;
	 allocnode* freed = nullptr;
	 allocnode* used = nullptr;
	 size_t capacity = 0;
	 size_t page = 0;
	 allocnode* allocations = nullptr;
	 char* heap = nullptr;
	 char* end = nullptr;
	 size_t total_allocations = 0;
      };
      template<typename T> inline bool bank::save(const char* in_filename, T in_buffer)
      {
	 FILE* file = fopen(in_filename, "wb");
	 if (file == nullptr) { return false; }
	 assert(in_buffer.head.ver == fourcc); // T::fourcc?
	 assert(in_buffer.data != nullptr);
	 // data can't point at header... unless it can!!
	 // turns out fw::Image immediately has the ibo, which is bad.
	 // assert(((blob::header*)in_buffer.data)->ver != fourcc);
	 // assert(((blob::header*)in_buffer.data) != &in_buffer.head);
	 if(in_buffer.head.hash == 0)
	 {
	    in_buffer.head.hash = hash::hash_buffer((const char*)in_buffer.data, sizeof(*in_buffer.data)*in_buffer.len);
	 }
	 fwrite(&in_buffer.head, sizeof(in_buffer.head), 1, file);
	 fwrite(in_buffer.data, sizeof(*in_buffer.data), in_buffer.len, file);
	 fclose(file);
	 return true;
      }
      template<typename T> inline bool bank::load(const char* in_filename, T& out_buffer)
      {
	 FILE* file = fopen(in_filename, "rb");
	 if (file == nullptr) { return false; }
	 fseek(file, 0, SEEK_END);
	 out_buffer.len = ftell(file);
	 assert(((out_buffer.len-sizeof(out_buffer.head)) % sizeof(*out_buffer.data)) == 0);
	 fseek(file, 0, SEEK_SET);
	 allocation* alloc = allocate(out_buffer.len + 1);
	 out_buffer.data = (decltype(out_buffer.data))alloc->data;
	 // todo: consider a short read
	 fread((char*)out_buffer.data, out_buffer.len, 1, file);
	 out_buffer.head = *((header*)out_buffer.data);
	 alloc->head = *((header*)out_buffer.data);
	 // todo: this shift is a bit awkward if you're using the allocate function directly.
	 // ----: should all allocations always have the blob header?
	 out_buffer.data = (decltype(out_buffer.data)) ((char*)out_buffer.data+sizeof(out_buffer.head));
	 assert(out_buffer.head.ver == fourcc);
	 // todo: decide if a file with no content is ok: (out_buffer.len == sizeof(out_buffer.head)).
	 assert(out_buffer.head.hash != 0 || out_buffer.len == sizeof(out_buffer.head));
	 fclose(file);
	 out_buffer.len = (out_buffer.len-sizeof(out_buffer.head)) / sizeof(*out_buffer.data);
	 return true;
      }
      // todo: should this shift / populate the header?
      // ----: related to above todo in load.
      // template<typename T> inline bool bank::allocate(asset<T>& out_buffer)
      // {
      // 	 assert(out_buffer.data == nullptr);
      // 	 assert(out_buffer.len != 0);
      // 	 size_t alloc_size = sizeof(decltype(*out_buffer.data))*out_buffer.len;
      // 	 allocation* alloc = allocate(alloc_size+1);
      // 	 out_buffer.data = (decltype(out_buffer.data))alloc->data;
      // 	 return true;
      // }
      template<typename T> inline bool bank::free(asset<T>& in)
      {
	 if(free((char*)in.data-sizeof(in.head))) { in = {}; return true; }
	 return false;
      }
      template<typename T> inline bool bank::find(hash::u32 in_hash, asset<T>& out_buffer)
      {
	 if(in_hash == 0) return false;
	 allocnode* alloc = used;
	 while(alloc != nullptr)
	 {
	    // assert(alloc->alloc.head.hash != 0);
	    if(alloc->alloc.head.hash == in_hash)
	    {
	       out_buffer = *((asset<T>*)&alloc->alloc);
	       out_buffer.len = (out_buffer.len-sizeof(out_buffer.head)) / sizeof(*out_buffer.data);
	       return true;
	    }
	    alloc = alloc->next;
	 }
	 return false;
      }
      template<typename T> inline bool bank::fixup(asset<T>& out_buffer) { return find(out_buffer.head.hash, out_buffer); }
      extern bank miscbank;
      extern bank meshbank;
      extern bank imagebank;
   }
}
