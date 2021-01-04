#include "mpeg_reader.h"
#include "../../graphics/resources/bitmap.h"
#include <string>

#include <iostream>
#include <filesystem>
#include "../../utils/string_helpers.h"
namespace fs = std::filesystem;

// needed because ffmpeg is a pure C library.
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
// #include <libavutil/opt.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
// #include <libavutil/error.h>
}

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
// #todo: put this somewhere else
static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
		     const char *filename)
{
   FILE *f;
   int i;
   f = fopen(filename,"wb");
   fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
   for (i = 0; i < ysize; i++)
      fwrite(buf + i * wrap, 1, xsize, f);
   fclose(f);
}

void mpeg_reader::dump_screenshot(int _frame_number)
{
   std::string screenshot_str = std::string(filename);
   screenshot_str = screenshot_str.substr(0, screenshot_str.length()-4);
   //yuv->rgb conversion
   int ret = 0;
   while (!feof(in_file))
   {
      /* read raw data from the input file */
      // #todo: fix magic numbers
      uint8_t inbuf[4096+AV_INPUT_BUFFER_PADDING_SIZE];
      memset(inbuf + 4096, 0, AV_INPUT_BUFFER_PADDING_SIZE);
      size_t data_size = fread(inbuf, 1, 4096, in_file);
      
      if (!data_size)
      {
	 break;
      }
      /* use the parser to split the data into frames */
      uint8_t* data = inbuf;
      av_init_packet(packet);
      while (data_size > 0)
      {
	 ret = av_parser_parse2(parser, codec_context, &packet->data, &packet->size,
				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	 if (ret < 0)
	 {
	    fprintf(stderr, "Error while parsing\n");
	    exit(1);
	 }
	 data      += ret;
	 data_size -= ret;
	 //sometimes the parser doesn't put data in the packet, not sure why.
	 if (packet->size)
	 {
	    //func starts here
	    ret = avcodec_send_packet(codec_context, packet);
	    if (ret < 0)
	    {
	       char error_buf [AV_ERROR_MAX_STRING_SIZE] = {0};
	       av_strerror(ret, error_buf, AV_ERROR_MAX_STRING_SIZE);
	       fprintf(stderr, "Error decoding frame: %s\n", error_buf);
	       exit(1);
	    }
	    while(ret >= 0)
	    {
	       ret = avcodec_receive_frame(codec_context, frame);
	       if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	       {
		  break;
	       }
	       else if (ret < 0)
	       {
		  fprintf(stderr, "Error during decoding\n");
		  exit(1);
	       }
	 
	       static bool do_once = true;
	       static int count = 0;
	       count++;
	       if (ret >= 0 && do_once && count == _frame_number)
	       {
		  do_once = false;
		  printf("saving: %s\n", (screenshot_str + "pgm").c_str());
		  pgm_save(frame->data[0], frame->linesize[0],
			   frame->width, frame->height, (screenshot_str + "pgm").c_str());

		  av_image_alloc(rgb_frame->data, rgb_frame->linesize, codec_context->width, codec_context->height, AV_PIX_FMT_RGB24, 32);
		  // yuv to rgb24 causes the image to flip vertically, doing this prior to the conversion negates the effect
		  // taken from here:
		  // https://topic.alibabacloud.com/a/decoding-h264-to-rgb-using-ffmpeg_8_8_10243900.html
		  frame->data[0] += frame->linesize[0] * (codec_context->height-1);
		  frame->linesize[0] *= -1;
		  frame->data[1] += frame->linesize[1] * (codec_context->height/2 - 1);
		  frame->linesize[1] *= -1;
		  frame->data[2] += frame->linesize[2] * (codec_context->height/2 - 1);
		  frame->linesize[2] *= -1;

		  // #todo: there seems to be an issue with image quality during conversion.
		  //        test out SWS_X vs SWS_POINT etc. 
		  sws_context = sws_getCachedContext(sws_context,
						     codec_context->width,codec_context->height, AV_PIX_FMT_YUV420P,
						     codec_context->width,codec_context->height, AV_PIX_FMT_RGB24,
						     SWS_X, nullptr, nullptr, nullptr);

		  sws_scale(sws_context, frame->data, frame->linesize, 0,
			    frame->height, rgb_frame->data, rgb_frame->linesize);


		  int bmp_data_size = 3*frame->width*frame->height;
		  bitmap bmp(width, height, (signed char*)&rgb_frame->data[0][0], bmp_data_size);
		  printf("saving: %s\n", (screenshot_str + "bmp").c_str());
		  bmp.save((screenshot_str + "bmp").c_str());
	       }
	    }
	 }
      }
      av_packet_unref(packet);
   }
}
#ifdef UNIT_TEST
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
   reader.dump_screenshot(200);
}
#endif
