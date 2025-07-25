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
      bool bank::is_initialised()
      {
	 return heap != nullptr;
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
	 AllocNode* node = used;
	 while(node != nullptr)
	 {
	    if(node->alloc.data == allocation)
	    {
	       AllocNode* previous = freed;
	       freed = node;
	       node->next = previous;
	       return true;
	    }
	    node = node->next;
	 }
	 return false;
      }
      // todo: force these to be page sized under the hood.
      // ----: consistent sizes will help with splitting/fragmentation.
      Allocation* bank::allocate(size_t size)
      {
	 assert(heap != nullptr);
	 assert((end-heap) + size < capacity);
	 assert(total_allocations < capacity/page);
      
	 AllocNode* node = nullptr;
	 if(freed != nullptr)
	 {
	    node = freed;
	    AllocNode* prev = nullptr;
	    while(node != nullptr)
	    {
	       // todo: split.
	       if(node->alloc.len >= size)
	       {
		  if (prev != nullptr) prev->next = node->next;
		  return &node->alloc;
	       }
	       node = node->next;
	    }
	 }
      
	 if(node == nullptr)
	 {
	    node = &allocations[total_allocations++];
	    *node = {{{}, end, size}, nullptr};
	    end += size;
	    AllocNode* previous = used;
	    used = node;
	    node->next = previous;
	 }
      
	 return &node->alloc;
      }
      bank miscbank;
      // float get_percent_used() { return  ((float)(end-heap) / (float)capacity)*100.0f; }
   }
}
