#include "blob.h"
#include <cassert>
#include <cstddef>

#define GiBs << 30
#define MiBs << 20
#define KiBs << 10

namespace blob
{
   const size_t capacity = 30 MiBs;
   char* heap = nullptr;
   char* end = nullptr;
   void init()
   {
      if(heap == nullptr)
      {
	 heap = new char[capacity];
	 end = heap;
      }
   }
   void shutdown()
   {
      delete [] heap;
      end = nullptr, heap = nullptr;
   }
   char* allocate(size_t size)
   {
      assert(heap != nullptr);
      assert((end-heap) + size < capacity);
      char* out = end; end += size;
      return out;
   }
   // float get_percent_used() { return  ((float)(end-heap) / (float)capacity)*100.0f; }
}
