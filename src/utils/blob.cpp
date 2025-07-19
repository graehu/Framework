#include "blob.h"
#include <cassert>
#include <cstddef>
#include <string.h>

namespace blob
{
   void bank::init()
   {
      if(heap == nullptr)
      {
	 assert(end == nullptr);
	 heap = new char[capacity]; end = heap;
	 memset(&allocations, 0, sizeof(AllocNode)*max_allocations);
      }
   }
   void bank::shutdown()
   {
      delete [] heap;
      end = nullptr, heap = nullptr;
      total_allocations = 0;
      memset(&allocations, 0, sizeof(AllocNode)*max_allocations);
   }
   bool bank::free(char* allocation)
   {
      AllocNode* alloc = used;
      while(alloc != nullptr)
      {
	 if(alloc->alloc.data == allocation)
	 {
	    AllocNode* previous = freed;
	    freed = alloc;
	    alloc->next = previous;
	    return true;
	 }
	 alloc = alloc->next;
      }
      return false;
   }
   // todo: force these to be page sized under the hood.
   // ----: consistent sizes will help with splitting/fragmentation.
   char* bank::allocate(size_t size)
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
	 AllocNode* previous = used;
	 used = alloc;
	 alloc->next = previous;
      }
      
      return (char*)alloc->alloc.data;
   }
   bank miscbank;
   // float get_percent_used() { return  ((float)(end-heap) / (float)capacity)*100.0f; }
}
