#ifndef MPEG_READER_H
#define MPEG_READER_H

#include "stdint.h"
#include "stdio.h"

struct AVCodecContext;
struct SwsContext;
struct AVPacket;
struct AVFrame;
struct AVCodecParserContext;

class mpeg_reader
{
public:
   mpeg_reader(const char* _filename);
   ~mpeg_reader();
   // #todo: make a proper type for image data
   void dump_random_screenshot();
private:
   AVFrame* frame;
   AVFrame* rgb_frame;
   AVPacket* packet;
   AVCodecParserContext *parser;
   const char* filename;
   FILE* in_file;
   AVCodecContext* codec_context;
   SwsContext* sws_context;
   //
   unsigned int width; // in pixels
   unsigned int height;
   //
};

#endif//MPEG_READER_H
