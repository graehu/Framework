#include "encrypt.h"
#include "sha.c"

namespace net
{
  namespace encrypt
  {
    void SHA1(unsigned char sha[20], unsigned char* in_buffer, unsigned int buffer_size)
    {
      SHA1_CTX ctx;
      SHA1_Init(&ctx);
      SHA1_Update(&ctx, in_buffer, buffer_size);
      SHA1_Final(&sha[0], &ctx);
    }
    
  }
}
