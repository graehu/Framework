#ifndef MPEG_DECODER_H
#define MPEG_DECODER_H

#include "stdint.h"
#include "stdio.h"

struct AVCodecContext;
struct SwsContext;
struct AVPacket;
struct AVFrame;
struct AVCodecParserContext;

class mpeg_decoder
{
public:
   mpeg_decoder();
   ~mpeg_decoder();
   void parse_packet(void* in_packet, size_t in_size);
   unsigned int get_rgb_frame_width();
   unsigned int get_rgb_frame_height();
   uint8_t* get_rgb_frame();
private:
   void print_packet();
   AVFrame* frame;
   AVFrame* rgb_frame;
   AVPacket* packet;
private:
   AVCodecParserContext *parser;
   AVCodecContext* codec_context;
   SwsContext* sws_context;
};

#endif//MPEG_DECODER_H
