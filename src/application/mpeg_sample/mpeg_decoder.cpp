#include "mpeg_decoder.h"
// #include "../../graphics/resources/bitmap.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

mpeg_decoder::mpeg_decoder() :
   frame(nullptr),
   rgb_frame(nullptr),
   packet(nullptr),
   parser(nullptr),
   codec_context(nullptr),
   sws_context(nullptr)
{
   packet = new AVPacket();
   //allocate h264 codec
   auto codec_id = AV_CODEC_ID_H264;
   AVCodecID e_codec_id = static_cast<AVCodecID>(codec_id);
   auto* codec = avcodec_find_decoder(e_codec_id);
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

mpeg_decoder::~mpeg_decoder()
{
   avcodec_close(codec_context);
   av_free(codec_context);
   av_freep(&frame->data[0]);
   av_frame_free(&frame);
   av_freep(&rgb_frame->data[0]);
   av_frame_free(&rgb_frame);
   delete packet;
}

uint8_t* mpeg_decoder::get_rgb_frame() { return &rgb_frame->data[0][0]; }
unsigned int mpeg_decoder::get_rgb_frame_width() { return rgb_frame->width; }
unsigned int mpeg_decoder::get_rgb_frame_height() { return rgb_frame->height; }

bool mpeg_decoder::parse_packet(void* in_packet, size_t in_size)
{
   if(in_packet == nullptr || in_size <= 0)
      return false;

   uint8_t* data = (uint8_t*)in_packet;
   size_t data_size = in_size;
   av_init_packet(packet);
   while (data_size > 0)
   {
      int ret = av_parser_parse2(parser, codec_context, &packet->data, &packet->size,
				 data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
      if (ret < 0)
      {
	 fprintf(stderr, "Error while parsing\n");
	 return false;
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
	    break;
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
	       break;
	    }
	    // print_packet();
	    if (ret >= 0)
	    {
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

	       // int bmp_data_size = 3*frame->width*frame->height;
	       // bitmap bmp(rgb_frame->width, rgb_frame->height, (signed char*)&rgb_frame->data[0][0], bmp_data_size);
	       // bmp.save("decoded.pkg.bmp");
	    }
	 }
      }
   }
   av_packet_unref(packet);
   return true;
}




