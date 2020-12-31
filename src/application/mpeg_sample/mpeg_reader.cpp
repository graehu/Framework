#include "mpeg_reader.h"
#include "../../graphics/resources/bitmap.h"
#include <string>

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;


// needed because ffmpeg is a pure C library.
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <libavutil/error.h>
}

int randomy = 2;

mpeg_reader::mpeg_reader(const char* _filename) :
   frame(nullptr),
   packet(nullptr),
   filename(_filename),
   in_file(nullptr),
   codec_context(nullptr),
   sws_context(nullptr)
{
   packet = new AVPacket();
   //allocate h264 codec
   auto codec_id = AV_CODEC_ID_H264;
   AVCodecID e_codec_id = static_cast<AVCodecID>(codec_id);
   auto* codec = avcodec_find_decoder(e_codec_id);
   width = 640, height = 480;
   if (codec == nullptr)
   {
      fprintf(stderr, "Codec not found\n");
      exit(1);
   }
   parser = av_parser_init(codec->id);
   if (!parser)
   {
      fprintf(stderr, "parser not found\n");
      exit(1);
   }
   codec_context = avcodec_alloc_context3(codec);
   if (codec_context == nullptr)
   {
      fprintf(stderr, "Could not allocate video codec context\n");
      exit(1);
   }
   // av_opt_set(codec_context->priv_data, "preset", "slow", 0);
   // open codec #todo: error handling, ret < 0 get error strings.
   if (avcodec_open2(codec_context, codec, nullptr) < 0)
   {
      fprintf(stderr, "Could not open codec\n");
      exit(1);
   }
   std::string file_str = std::string(filename);
   printf("file: %s\n", filename);
   in_file = fopen(filename, "rb");
   if (in_file == nullptr)
   {
      fprintf(stderr, "Could not open %s\n", file_str.c_str());
      exit(1);
   }
   //allocate frames
   frame = av_frame_alloc();
   if (frame == nullptr)
   {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
   }
   rgb_frame = av_frame_alloc();
   if (rgb_frame == nullptr)
   {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
   }
}
mpeg_reader::~mpeg_reader()
{
   fclose(in_file);
   avcodec_close(codec_context);
   av_free(codec_context);
   av_freep(&frame->data[0]);
   av_frame_free(&frame);
   av_freep(&rgb_frame->data[0]);
   av_frame_free(&rgb_frame);
   delete packet;
}

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
		     char *filename)
{
   FILE *f;
   int i;
   // #todo: potentially need this to be wb not w
   f = fopen(filename,"wb");
   fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
   for (i = 0; i < ysize; i++)
      fwrite(buf + i * wrap, 1, xsize, f);
   fclose(f);
}
// static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
// {
//    int ret = avcodec_send_packet(dec_ctx, pkt);
//    if (ret < 0)
//    {
//       fprintf(stderr, "Error sending a packet for decoding\n");
//       exit(1);
//    }
//    while (ret >= 0)
//    {
//       ret = avcodec_receive_frame(dec_ctx, frame);
//       if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
//       {
// 	 return;
//       }
//       else if (ret < 0)
//       {
// 	 fprintf(stderr, "Error during decoding\n");
// 	 exit(1);
//       }
//       /* the picture is allocated by the decoder. no need to
// 	 free it */
//       const char* filename = "screen.pgm";
//       pgm_save(frame->data[0], frame->linesize[0],
// 	       frame->width, frame->height, (char*)filename);
//    }
// }
//https://topic.alibabacloud.com/a/decoding-h264-to-rgb-using-ffmpeg_8_8_10243900.html
// int H264_2_RGB(unsigned char *inputbuf, int frame_size, unsigned char *outputbuf, unsigned int*outsize)
// {
// 	int             decode_size;
// 	int             numBytes;
// 	int             av_result;
// 	uint8_t         *buffer = NULL;
 
// 	printf("Video decoding\n");
 
// 	av_result = avcodec_decode_video(pCodecCtx, pFrame, &decode_size, inputbuf, frame_size);
// 	if (av_result < 0)
// 	{
// 		fprintf(stderr, "decode failed: inputbuf = 0x%x , input_framesize = %d\n", inputbuf, frame_size);
// 		return -1;
// 	}
 
// 	// Determine required buffer size and allocate buffer
// 	numBytes=avpicture_get_size(PIX_FMT_BGR24, pCodecCtx->width,
// 		pCodecCtx->height);
// 	buffer = (uint8_t*)malloc(numBytes * sizeof(uint8_t));
// 	// Assign appropriate parts of buffer to image planes in pFrameRGB
// 	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_BGR24,
// 		pCodecCtx->width, pCodecCtx->height);
 
// 	img_convert_ctx = sws_getCachedContext(img_convert_ctx,pCodecCtx->width,pCodecCtx->height,
// 		//PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
// 		pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height,PIX_FMT_RGB24 ,
// 		SWS_X ,NULL,NULL,NULL) ;
// 	if (img_convert_ctx == NULL) 
// 	{
 
// 		printf("can't init convert context!\n") ;
// 		return -1;
// 	}
// 	pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height-1);
// 	pFrame->linesize[0] *= -1;
// 	pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height/2 - 1);;
// 	pFrame->linesize[1] *= -1;
// 	pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height/2 - 1);;
// 	pFrame->linesize[2] *= -1;
// 	sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,
// 		0, 0 - pCodecCtx->width, pFrameRGB->data, pFrameRGB->linesize);
	
// 	if (decode_size)
// 	{
// 		*outsize = pCodecCtx->width * pCodecCtx->height * 3;
// 		memcpy(outputbuf, pFrameRGB->data[0], *outsize);
// 	}
//   	free(buffer);
// 	return 0;
// }


   // int ret;
   // do
   // {
   //    fflush(stdout);
   //    ret = avcodec_receive_packet(codec_context, packet);
   //    if (ret >= 0)
   //    {
   // 	 av_packet_unref(packet);
   //    }
   //    else if (ret == AVERROR_EOF)
   //    {
   // 	 break;
   //    }
   //    else if(ret < 0 && ret != AVERROR(EAGAIN))
   //    {
   // 	 char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
   // 	 av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
   // 	 fprintf(stderr, "Error encoding frame %s\n", error_buf);
   // 	 exit(1);
   //    }
   // } while (ret == EAGAIN);
   
void mpeg_reader::dump_random_screenshot()
{
   //yuv->rgb conversion
   int ret = 0;
   while (!feof(in_file))
   {
      /* read raw data from the input file */
      // #todo: fix magic numbers
#define INBUF_SIZE 4096
      uint8_t inbuf[INBUF_SIZE+AV_INPUT_BUFFER_PADDING_SIZE];
      memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
      size_t data_size = fread(inbuf, 1, INBUF_SIZE, in_file);
      if (!data_size)
	 break;
   
      /* use the parser to split the data into frames */
      uint8_t* data = inbuf;
      
      while (data_size > 0)
      {
	 av_init_packet(packet);
	 // packet->data = nullptr;
	 // packet->size = 0;
	 // frame->pts++;
	 ret = av_parser_parse2(parser, codec_context, &packet->data, &packet->size,
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	 if (ret < 0)
	 {
	    fprintf(stderr, "Error while parsing\n");
	    exit(1);
	 }
	 data      += ret;
	 data_size -= ret;
	 ret = avcodec_send_packet(codec_context, packet);
	 if (ret < 0)
	 {
	    char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
	    av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
	    fprintf(stderr, "Error decoding frame: %s\n", error_buf);
	    exit(1);
	 }
	 int fytest = 0;
	 while(ret >= 0)
	 {
	    fytest++;
	    printf("%d\n", fytest);
	    ret = avcodec_receive_frame(codec_context, frame);
	    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	       break;
	    else if (ret < 0) {
	       fprintf(stderr, "Error during decoding\n");
	       exit(1);
	    }
	 
	    static bool do_once = true;
	    static int count = 0;
	    count++;
	    // printf("%d\n", count);
	    if (ret >= 0 && do_once && count == 20)
	    {
	       printf("doing it\n");
	       do_once = false;
	       pgm_save(frame->data[0], frame->linesize[0],
			frame->width, frame->height, (char*)"out.pgm");

	       av_image_alloc(rgb_frame->data, rgb_frame->linesize, codec_context->width, codec_context->height, AV_PIX_FMT_RGB24, 32);
	       // yuv to rgb24 causes the image to flip vertically, doing this prior to the conversion negates the effect
	       frame->data[0] += frame->linesize[0] * (codec_context->height-1);
	       frame->linesize[0] *= -1;
	       frame->data[1] += frame->linesize[1] * (codec_context->height/2 - 1);
	       frame->linesize[1] *= -1;
	       frame->data[2] += frame->linesize[2] * (codec_context->height/2 - 1);
	       frame->linesize[2] *= -1;
	       
	       sws_context = sws_getCachedContext(sws_context,
						  codec_context->width,codec_context->height, AV_PIX_FMT_YUV420P,
						  codec_context->width,codec_context->height, AV_PIX_FMT_RGB24,
						  SWS_X, nullptr, nullptr, nullptr);

	       sws_scale(sws_context, frame->data, frame->linesize, 0,
			 frame->height, rgb_frame->data, rgb_frame->linesize);


	       int data_size = 3*frame->width*frame->height;
	       bitmap bmp(width, height, (signed char*)&rgb_frame->data[0][0], data_size);
	       bmp.save("out.bmp");
	    }
	 }
	 av_packet_unref(packet);
      }
   }
   printf("leaving\n");
}

int main(int argc, char *argv[])
{
   const char *filename = argv[1];
   std::cout << "Current path is " << fs::current_path() << '\n';
   printf("%d arguments: \n", argc);
   for(int i = 0; i < argc; i++)
   {
      printf("\t%d: %s\n", i, argv[i]);
   }
   printf("reading: %s\n", filename);
   mpeg_reader reader(filename);
   reader.dump_random_screenshot();
}

/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
    
/**
 * @file
 * video decoding with libavcodec API example
 *
 * @example decode_video.c
 */
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C"
{    
#include <libavcodec/avcodec.h>
}
    
#define INBUF_SIZE 4096

// #note: this is the part where the format is written, no encoder needed.
// static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
// 		     char *filename)
// {
//    FILE *f;
//    int i;
//    // #todo: potentially need this to be wb not w
//    f = fopen(filename,"wb");
//    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
/// //    for (i = 0; i < ysize; i++)
//       fwrite(buf + i * wrap, 1, xsize, f);
//    fclose(f);
// }
    
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
		   const char *filename)
{
   char buf[1024];
   int ret;
   ret = avcodec_send_packet(dec_ctx, pkt);
   if (ret < 0) {
      fprintf(stderr, "Error sending a packet for decoding\n");
      exit(1);
   }
    
   while (ret >= 0) {
      ret = avcodec_receive_frame(dec_ctx, frame);
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	 return;
      else if (ret < 0) {
	 fprintf(stderr, "Error during decoding\n");
	 exit(1);
      }
    
      printf("saving frame %3d\n", dec_ctx->frame_number);
      fflush(stdout);
    
      /* the picture is allocated by the decoder. no need to
	 free it */
      snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
      pgm_save(frame->data[0], frame->linesize[0],
	       frame->width, frame->height, buf);
   }
}
    
int main2(int argc, char **argv)
{
   const char *filename, *outfilename;
   const AVCodec *codec;
   AVCodecParserContext *parser;
   AVCodecContext *c= NULL;
   FILE *f;
   AVFrame *frame;
   uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
   uint8_t *data;
   size_t   data_size;
   int ret;
   AVPacket *pkt;
   if (argc <= 2) {
      fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
      exit(0);
   }
   filename    = argv[1];
   outfilename = argv[2];
   
   pkt = av_packet_alloc();
   if (!pkt)
      exit(1);
   
   /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
   memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
   
   /* find the MPEG-1 video decoder */
   codec = avcodec_find_decoder(AV_CODEC_ID_H264);
   if (!codec) {
      fprintf(stderr, "Codec not found\n");
      exit(1);
   }
   parser = av_parser_init(codec->id);
   if (!parser) {
      fprintf(stderr, "parser not found\n");
      exit(1);
   }
   c = avcodec_alloc_context3(codec);
   if (!c) {
      fprintf(stderr, "Could not allocate video codec context\n");
      exit(1);
   }
   /* For some codecs, such as msmpeg4 and mpeg4, width and height
      MUST be initialized there because this information is not
      available in the bitstream. */
   
   /* open it */
   if (avcodec_open2(c, codec, NULL) < 0) {
      fprintf(stderr, "Could not open codec\n");
      exit(1);
   }
   f = fopen(filename, "rb");
   if (!f) {
      fprintf(stderr, "Could not open %s\n", filename);
      exit(1);
   }
   frame = av_frame_alloc();
   if (!frame) {
      fprintf(stderr, "Could not allocate video frame\n");
      exit(1);
   }
   while (!feof(f))
   {
      /* read raw data from the input file */
      data_size = fread(inbuf, 1, INBUF_SIZE, f);
      if (!data_size)
   	 break;
   
      /* use the parser to split the data into frames */
      data = inbuf;
      while (data_size > 0)
      {
   	 ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
   				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
   	 if (ret < 0) {
   	    fprintf(stderr, "Error while parsing\n");
   	    exit(1);
   	 }
   	 data      += ret;
   	 data_size -= ret;
   
   	 if (pkt->size)
   	    decode(c, frame, pkt, outfilename);
      }
   }
   /* flush the decoder */
   decode(c, frame, NULL, outfilename);
  
   fclose(f);
  
   av_parser_close(parser);
   avcodec_free_context(&c);
   av_frame_free(&frame);
   av_packet_free(&pkt);
   return 0;
}
