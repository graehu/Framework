#include "bitmap.h"

#include <stdio.h>
#include <cassert>
#include <cmath>
#include <cstdlib>

// compression table
// Value Identified by	Compression method	| Comments
// 0	| BI_RGB	none	                | Most common
// 1	| BI_RLE8	RLE 8-bit/pixel         | Can be used only with 8-bit/pixel bitmaps
// 2	| BI_RLE4	RLE 4-bit/pixel	        | Can be used only with 4-bit/pixel bitmaps


#pragma pack(1)
struct RGB 
{ 
   signed char blue;
   signed char green;
   signed char red;
   signed char alpha;
};

struct BMPInfoHeader 
{
   unsigned int	size;
   unsigned int	width;
   unsigned int	height;
   unsigned short  planes;
   unsigned short  bits;
   unsigned int	compression;
   unsigned int	imageSize;
   unsigned int	xScale;
   unsigned int	yScale;
   unsigned int	colors;
   unsigned int	importantColors;
};

struct BMPHeader 
{
   unsigned short type; 
   unsigned int size; 
   unsigned short unused; 
   unsigned short unused2; 
   unsigned int offset; 
}; 

struct BMPInfo 
{
   BMPInfoHeader		header;
   RGB				colors[1];
};
#pragma pack(8)

//taken from glTools and heavily modified.
bitmap::bitmap(const char* _filename)
{
   m_width = 0;
   m_height = 0;

   FILE*	pFile;
   BMPInfo *pBitmapInfo = nullptr;
   unsigned long lInfoSize = 0;
   unsigned long lBitSize = 0;
   m_data = nullptr;					// Bitmaps bits
   BMPHeader	bitmapHeader;
   m_loaded = false;

   // Attempt to open the file
   pFile = fopen(_filename, "rb");
   if(pFile == nullptr)
   {
      printf("failed to open %s", _filename);
      return;
   }

   // File is Open. Read in bitmap header information
   fread(&bitmapHeader, sizeof(BMPHeader), 1, pFile);
   //test
   // Read in bitmap information structure
   lInfoSize = bitmapHeader.offset - sizeof(BMPHeader);
   pBitmapInfo = (BMPInfo*) new signed char[lInfoSize];

   if(fread(pBitmapInfo, lInfoSize, 1, pFile) != 1)
   {
      printf("failed to read bitmapinfo from %s", _filename);
      delete pBitmapInfo;
      fclose(pFile);
      assert(false);
      return;
   }

   // Save the size and dimensions of the bitmap
   m_width = pBitmapInfo->header.width;
   m_height = pBitmapInfo->header.height;
   lBitSize = pBitmapInfo->header.imageSize;

   // If the size isn't specified, calculate it anyway
   if(pBitmapInfo->header.bits != 24)
   {
      printf("%dbit bitmaps are not supported, supply a 24bit bitmap\n", pBitmapInfo->header.bits);
      delete pBitmapInfo;
      return;
   }

   //fallback
   if(lBitSize == 0)
   {
      lBitSize = (m_width * pBitmapInfo->header.bits + 7) / 8 * abs(m_height);
   }

   // Allocate space for the actual bitmap
   delete pBitmapInfo;
   m_data = new signed char[lBitSize];// (sizeof(byte)*lBitSize);

   // Read in the bitmap bits, check for corruption
   if(fread(m_data, lBitSize, 1, pFile) != 1)
   {
      delete m_data;
      m_data = nullptr;
      m_data_size = 0;
   }
   m_data_size = lBitSize;
   
   // Close the bitmap file now that we have all the data we need
   fclose(pFile);
   m_loaded = true;
   m_filename = _filename;
}//*/
bitmap::bitmap(int _width, int _height, signed char* _data, int _data_size)
{
   m_loaded = true;
   m_width = _width;
   m_data = _data;
   m_data_size = _data_size;
   m_height = _height;
   m_filename = nullptr;
}

// lpBits stand for long point bits

// szPathName : Specifies the pathname        -> the file path to save the image
// lpBits    : Specifies the bitmap bits      -> the buffer (content of the) image
// w    : Specifies the image width
// h    : Specifies the image height
bool bitmap::save(const char* _filename)
{
   // Create a new file for writing
   FILE*	pFile;
   pFile = fopen(_filename, "wb");
   if(pFile == nullptr)
   {
      printf("failed to open %s", _filename);
      return false;
   }
   
   BMPInfoHeader bmih;
   bmih.size = sizeof(BMPInfoHeader);
   bmih.width = m_width;
   bmih.height = m_height;
   bmih.planes = 1;
   bmih.bits = 24;
   bmih.compression = 0;
   bmih.imageSize = m_width * m_height * 3;

   BMPHeader bmfh;
   int nBitsOffset = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
   bmfh.type = 'B' + ('M' << 8);
   bmfh.offset = nBitsOffset;
   bmfh.size = nBitsOffset + bmih.imageSize;;
   bmfh.unused = bmfh.unused = 0;

   // Write the bitmap file header
   fwrite((const char*)&bmfh, 1, sizeof(BMPHeader), pFile);
   // pFile.write((const char*)&bmfh, sizeof(BMPHeader));

   // And then the bitmap info header
   fwrite((const char*)&bmih, 1, sizeof(BMPInfoHeader), pFile);
   // pFile.write((const char*)&bmih, sizeof(BMPInfoHeader));

   // Finally, write the image data itself
   //-- the data represents our drawing
   fwrite(&m_data[0], 1, m_data_size, pFile);
   // pFile.write(&m_data[0], lpBits.size());
   fclose(pFile);

   return true;
}


signed char bitmap::get_red_value(unsigned int _u, unsigned int _v)
{
   signed char red = 0;
   if((_u*3)+(_v*m_width*3)+2 > m_width*m_height*3)
   {
      return 0;
   }
		
   red = m_data[(_u*3)+(_v*m_width*3) + 2];
   return red;
}

bitmap::~bitmap()
{
   if(m_data != nullptr && m_filename != nullptr)
   {
      delete m_data;
   }
}
