#ifndef ENCRYPT_H
#define ENCRYPT_H

namespace net
{
  namespace encrypt
  {
    void SHA1(unsigned char sha[20], unsigned char* in_buffer, unsigned int buffer_size);
  }
}

#endif//ENCRYPT_H
