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
	       if(freed > node || freed == nullptr)
	       {
		  node->next = freed;
		  freed = node;
	       }
	       else
	       {
		  node->next = freed->next;
		  freed->next = node;
	       }
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
	 // quantise to page size
	 size = ((size/page)+1)*page;
	 log::debug("allocating {}", size);
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
	       // note: this goes horribly if the bank is allocating lots of different sized things
	       // ----: splitting also goes poorly if that's the case.
	       // ----: so, only try to use freed if the slop isn't massive, 8 pages or 16 pages.
	       if(node->alloc.len >= size && ((node->alloc.len/page)-(size/page)) < 8)
	       {
		  log::debug("found free alloc {} vs {}", (size/page), node->alloc.len/page);
		  // if((size/page) != (node->alloc.len/page))
		  // {
		  //    log::debug("splitting");
		  //    allocnode* split = &allocations[total_allocations++];
		  //    *split = {{{}, node->alloc.data, size}, node};
		  //    node->alloc.data += size;
		  //    node->alloc.len -= size;
		  //    node = split;
		  //    freecount++;
		  // }
		  if (prev != nullptr) { prev->next = node->next; }
		  if(node == freed) { freed = freed->next; }
		  if(used > node || used == nullptr)
		  {
		     node->next = used;
		     used = node;
		  }
		  else
		  {
		     node->next = used->next;
		     used->next = node;
		  }
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
      void bank::print()
      {
	 log::scope topic("blob");
	 allocnode* node = used;
	 unsigned int node_count = 0;
	 while(node != nullptr)
	 {
	    node_count++;
	    node = node->next;
	 }
	 log::debug("actual used: {} vs {}", node_count, usedcount);
	 node = freed;
	 node_count = 0;
	 while(node != nullptr)
	 {
	    node_count++;
	    node = node->next;
	 }
	 log::debug("actual freed: {} vs {}", node_count, freecount);
      }
      bank miscbank;
      bank meshbank;
      bank imagebank;
   }
}
