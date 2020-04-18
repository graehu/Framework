#include "encode.h"
namespace net
{
  namespace encode
  {
    void Base64(unsigned char* pOut, unsigned char* pIn, size_t nLen)
    {
      static const char* CODES ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
      //..straight from wikipedia.
      int b;
      unsigned char* o = pOut;
      for(size_t i = 0; i < nLen; i += 3)
	{
	  b = (pIn[i] & 0xfc) >> 2;
	  *o++ = CODES[b];
	  b = (pIn[i] & 0x3) << 4;
	  if(i + 1 < nLen)
	    {
	      b |= (pIn[i + 1] & 0xF0) >> 4;
	      *o++ = CODES[b];
	      b = (pIn[i + 1] & 0x0F) << 2;
	      if (i + 2 < nLen)
		{
		  b |= (pIn[i + 2] & 0xC0) >> 6;
		  *o++ = CODES[b];
		  b = pIn[i + 2] & 0x3F;
		  *o++ = CODES[b];
		} 
	      else
		{
		  *o++ = CODES[b];
		  *o++ = '=';
		}
	    }
	  else
	    {
	      *o++ = CODES[b];
	      *o++ = '=';
	      *o++ = '=';
	    }
	}
    }
    
  }
}
