#include <stdio.h>
#include <iostream>
#include <inttypes.h>
#include "mpeg_decoder.h"
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

unsigned char *byteBuffer = new unsigned char[256];
size_t bufferLength = 256;

int *s_image_addr = nullptr;
int s_image_len = 0;
int *s_packet_addr = nullptr;
int s_packet_len = 0;

val getImageBuffer()
{
   return val(typed_memory_view(s_image_len, s_image_addr));
}
EMSCRIPTEN_BINDINGS(functions_get_image) { function("getImageBuffer", &getImageBuffer); }

val getPacketBuffer()
{
    return val(typed_memory_view(s_packet_len, s_packet_addr));
}
EMSCRIPTEN_BINDINGS(functions_get_packet) { function("getPacketBuffer", &getPacketBuffer); }

mpeg_decoder decoder;

void testing(int32_t* in_addr, int len)
{
   std::cout << "addr: " << in_addr << " len: " << len << '\n';
}

extern "C"
{
   void init_heaps(int image_len, int packet_len)
   {
      if(image_len > s_image_len)
      {
	 if(s_image_addr != nullptr)
	 {
	    delete [] s_image_addr;
	 }
	 s_image_addr = new int[image_len];
	 s_image_len = image_len;
      }
      if(packet_len > s_packet_len)
      {
	 if(s_packet_addr != nullptr)
	 {
	    delete [] s_packet_addr;
	 }
	 s_packet_addr = new int[packet_len];
	 s_packet_len = packet_len;
      }
   }
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
   }
   bool decode_packet(int len)
   {
      if(len > s_packet_len)
      {
	 decoder.parse_packet(s_packet_addr, len);
	 int frame_width = decoder.get_rgb_frame_width();
	 int frame_height = decoder.get_rgb_frame_height();
	 int frame_len = frame_height*frame_width*3;
	 if(frame_len <= s_image_len)
	 {
	    auto frame = decoder.get_rgb_frame();
	    for (int32_t i = 0; i < s_image_len; i++)
	    {
	       s_packet_addr[i] = frame[i];
	    }
	    return true;
	 }
	 else
	 {
	    printf("error: decoded frame was %d buffer was %d", frame_len, s_image_len);
	 }
      }
      else
      {
	 printf("error: packet was %d buffer was %d", len, s_packet_len);
      }
      return false;
   }
}
