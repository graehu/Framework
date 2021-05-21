#include <stdio.h>
#include <iostream>
#include <inttypes.h>

void testing(int32_t* in_addr, int len)
{
   std::cout << "addr: " << in_addr << " len: " << len << '\n';
}

extern "C"
{
   void fillArray(int32_t* addr, int len)
   {
      testing(addr, len);
      for (int32_t i = 0; i < len; i++)
      {
	 if(i % 3 == 0)
	 {
	    addr[i] = 0xff0000ff;
	 }
	 else if(i % 3 == 1)
	 {
	    addr[i] = 0x00ff00ff;
	 }
	 else if(i % 3 == 2)
	 {
	    addr[i] = 0x0000ffff;
	 }
      }
      // for (int32_t j = 0; j < 8; ++j)
      // {
      // 	 unsigned int r = (addr[j] & 0xff000000) >> 24;
      // 	 unsigned int g = (addr[j] & 0x00ff0000) >> 16;
      // 	 unsigned int b = (addr[j] & 0x0000ff00) >> 8;
      // 	 unsigned int a = (addr[j] & 0x000000ff);
      // 	 printf("a[%d] = r: %d g: %d b: %d a: %d\n", j, r, g, b, a);
      // }
   }
}
