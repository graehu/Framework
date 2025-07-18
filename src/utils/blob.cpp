#include "blob.h"
#include <cassert>
#include <cstddef>
#include <string.h>

#define GiBs << 30
#define MiBs << 20
#define KiBs << 10

namespace blob
{
   typedef Buffer<char> Allocation;
   struct AllocNode
   {
      Allocation alloc;
      AllocNode* next = nullptr;
   };
   const size_t capacity = 1 GiBs;
   const size_t max_allocations = 1 MiBs;
   AllocNode allocations[max_allocations];
   char* heap = nullptr;
   char* end = nullptr;
   AllocNode* freed = nullptr;
   AllocNode* used = nullptr;
   size_t total_allocations = 0;
   void init()
   {
      if(heap == nullptr)
      {
	 assert(end == nullptr);
	 heap = new char[capacity]; end = heap;
	 memset(&allocations, 0, sizeof(AllocNode)*max_allocations);
      }
   }
   void shutdown()
   {
      delete [] heap;
      end = nullptr, heap = nullptr;
      total_allocations = 0;
      memset(&allocations, 0, sizeof(AllocNode)*max_allocations);
   }
   bool free(char* allocation)
   {
      AllocNode* alloc = used;
      while(alloc != nullptr)
      {
	 if(alloc->alloc.data == allocation)
	 {
	    // found it.
	    // add to the freelist.
	    if (freed == nullptr)
	    {
	       freed = alloc;
	       alloc->next = nullptr;
	    }
	    else
	    {
	       auto prev = freed->next;
	       freed->next = alloc;
	       alloc->next = prev;
	    }
	    return true;
	 }
	 alloc = alloc->next;
      }
      return false;
   }
   // todo: force these to be page sized under the hood.
   // ----: consistent sizes will help with splitting/fragmentation.
   char* allocate(size_t size)
   {
      assert(heap != nullptr);
      assert((end-heap) + size < capacity);
      assert(total_allocations < max_allocations);
      
      AllocNode* alloc = nullptr;
      if(freed != nullptr)
      {
	 alloc = freed;
	 AllocNode* prev = nullptr;
	 while(alloc != nullptr)
	 {
	    // todo: split.
	    if(alloc->alloc.len >= size)
	    {
	       if (prev != nullptr) prev->next = alloc->next;
	       return (char*)alloc->alloc.data;
	    }
	    alloc = alloc->next;
	 }
      }
      
      if(alloc == nullptr)
      {
	 alloc = &allocations[total_allocations++];
	 *alloc = {{end, size}, nullptr};
	 end += size;
	 if(used == nullptr) { used = alloc; }
	 else
	 {
	    AllocNode* previous = used->next;
	    used->next = alloc;
	    alloc->next = previous;
	 }
      }
      
      return (char*)alloc->alloc.data;
   }
   // float get_percent_used() { return  ((float)(end-heap) / (float)capacity)*100.0f; }
}
