#include "blob.h"
#include <cassert>
#include <cstddef>
#include <string.h>
#include "log/log.h"
namespace fw
{    
   namespace blob
   {
      void bank::init(size_t in_capacity, size_t in_page)
      {
	 if(heap == nullptr)
	 {

	    fw::log::topics::add("blob");
	    assert(end == nullptr);
	    assert(in_capacity > 1 KiBs);
	    assert((in_capacity % in_page) == 0);
	    assert(in_page >= sizeof(int));
	    capacity = in_capacity;
	    page = in_page;
	    used = nullptr;
	    freed = nullptr;
	    heap = new char[capacity]; end = heap;
	    allocations = new allocnode[capacity/page];
	    memset(allocations, 0, sizeof(allocnode)*capacity/page);
	 }
      }
      void bank::shutdown()
      {
	 delete [] heap;
	 memset(allocations, 0, sizeof(allocnode)*capacity/page);
	 delete [] allocations;
	 end = nullptr, heap = nullptr;
	 total_allocations = 0;
      }
      
      bool bank::free(char* allocation)
      {
	 log::scope topic("blob");
	 log::debug("freeing");
	 allocnode* node = used;
	 allocnode* previous = nullptr;
	 while(node != nullptr)
	 {
	    if(node->alloc.data == allocation)
	    {
	       if(previous) {previous->next = node->next;}
	       if(node == used) { used = used->next; }
	       node->next = freed;
	       freed = node;
	       log::debug("freed");
	       freecount++;
	       usedcount--;
	       return true;
	    }
	    previous = node;
	    node = node->next;
	 }
	 return false;
      }
      // todo: force these to be page sized under the hood.
      // ----: consistent sizes will help with splitting/fragmentation.
      allocation* bank::allocate(size_t size)
      {
	 log::scope topic("blob");
	 log::debug("allocating");
	 assert(heap != nullptr);
	 assert((end-heap) + size < capacity);
	 assert(total_allocations < capacity/page);
      
	 allocnode* node = nullptr;
	 if(freed != nullptr)
	 {
	    node = freed;
	    allocnode* prev = nullptr;
	    while(node != nullptr)
	    {
	       // todo: split.
	       if(node->alloc.len >= size)
	       {
		  log::debug("found free alloc");
		  if (prev != nullptr) { prev->next = node->next; }
		  if(node == freed) { freed = freed->next; }
		  node->next = used;
		  used = node;
		  freecount--;
		  usedcount++;
		  break;
	       }
	       prev = node;
	       node = node->next;
	    }
	 }
	 log::debug("need new alloc");
	 if(node == nullptr)
	 {
	    usedcount++;
	    node = &allocations[total_allocations++];
	    *node = {{{}, end, size}, nullptr};
	    end += size;
	    allocnode* previous = used;
	    used = node;
	    node->next = previous;
	 }
      
	 return &node->alloc;
      }
      bank miscbank;
   }
}
