#include <stdio.h>
#include <iostream>
#include <inttypes.h>

void testing()
{
   std::cout << "it does work!" << '\n';
}

extern "C"
{
   void fillArray(int32_t* a, int len)
   {
      testing();
      for (int32_t i = 0; i < len; i++)
      {
	 a[i] = i * i;
      }
      for (int32_t j = 0; j < len; ++j)
      {
	 printf("a[%d] = %d\n", j, a[j]);
      }
   }
}
