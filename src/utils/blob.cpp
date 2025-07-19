#include "blob.h"
#include <cassert>
#include <cstddef>
#include <string.h>
namespace fw
{    
   namespace blob
   {
      void bank::init(size_t in_capacity, size_t in_page)
      {
	 if(heap == nullptr)
	 {
	    assert(end == nullptr);
	    assert(in_capacity > 1 KiBs);
	    assert((in_capacity % in_page) == 0);
	    assert(in_page >= sizeof(int));
	    capacity = in_capacity;
	    page = in_page;
	    heap = new char[capacity]; end = heap;
	    allocations = new AllocNode[capacity/page];
	    memset(allocations, 0, sizeof(AllocNode)*capacity/page);
	 }
      }
      void bank::shutdown()
      {
	 delete [] heap;
	 memset(allocations, 0, sizeof(AllocNode)*capacity/page);
	 delete [] allocations;
	 end = nullptr, heap = nullptr;
	 total_allocations = 0;
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
	 assert(total_allocations < capacity/page);
      
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
}
