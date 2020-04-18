#ifndef ENCODE_H
#define ENCODE_H
#include <cstddef>
namespace net
{
  namespace encode
  {
    void Base64(unsigned char* pOut, unsigned char* pIn,  size_t nLen);
  }
}
#endif//ENCODE_H


