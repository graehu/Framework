#include "bitmap.h"

#include <stdio.h>
#include <cassert>
#include <cmath>
#include <cstdlib>

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
	BMPInfo *pBitmapInfo = NULL;
	unsigned long lInfoSize = 0;
	unsigned long lBitSize = 0;
	m_data = NULL;					// Bitmaps bits
	BMPHeader	bitmapHeader;

	// Attempt to open the file
    pFile = fopen(_filename, "rb");
    if(pFile == NULL)
	{
		assert(false);
	}

	// File is Open. Read in bitmap header information
    fread(&bitmapHeader, sizeof(BMPHeader), 1, pFile);

	
	//test

	// Read in bitmap information structure
	lInfoSize = bitmapHeader.offset - sizeof(BMPHeader);
	pBitmapInfo = (BMPInfo*) new signed char[lInfoSize];

	if(fread(pBitmapInfo, lInfoSize, 1, pFile) != 1)
	{
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
		delete pBitmapInfo;
		assert(false);
		return;
	}
	
	if(lBitSize == 0)
	lBitSize = 		lBitSize = (m_width *
           pBitmapInfo->header.bits + 7) / 8 *
  		  abs(m_height);

	// Allocate space for the actual bitmap
	delete pBitmapInfo;
	m_data = new signed char[lBitSize];// (sizeof(byte)*lBitSize);

		// Read in the bitmap bits, check for corruption
	if(fread(m_data, lBitSize, 1, pFile) != 1)
	{
		delete m_data;
		m_data = NULL;
	}
	
	// Close the bitmap file now that we have all the data we need
	fclose(pFile);
}//*/

signed char bitmap::getRedVal(unsigned int _u, unsigned int _v)
{
	signed char red = 0;
	if((_u*3)+(_v*m_width*3)+2 > m_width*m_height*3) return 0;
		
	red = m_data[(_u*3)+(_v*m_width*3) + 2];
	return red;
	return 0;
}

bitmap::~bitmap()
{
	if(m_data != NULL)
		delete m_data;
}