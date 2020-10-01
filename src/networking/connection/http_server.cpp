#include "http_server.h"
#include "../packet/packet.h"
#include "address.h"
#include "socket.h"
#include "../utils/encrypt.h"
#include "../utils/encode.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include "../../utils/log/log.h"
#include "../../utils/log/log_macros.h"
// lib includes
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <ios>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <sstream>
#include <thread>
#include <chrono>
#include <type_traits>

using namespace fw;

net::http_server::http_server(unsigned int port) :
   mv_handler(nullptr),
   mv_server_thread(nullptr),
   mv_running(true)
   
{
   static bool do_once = true;
   if(do_once)
   {
      log::topics::add("websocket");
      log::topics::add("http_server");
      do_once = false;
   }
      
   net::socket lv_listen_socket(net::socket::eHttpSocket);
   for(int i = 0; !lv_listen_socket.openSock(port+i) && i < 100; i++) { }
   //todo: find out exactly how unsafe this is.
   lv_listen_socket.mf_set_keepalive(true);
   mv_server_thread = new std::thread(&net::http_server::mf_server_thread, this, lv_listen_socket);
}
namespace net
{
   //todo: test a smaller packet.
   typedef NewPacket<33068> http_packet;
   // typedef NewPacket<16384> http_packet;
   // typedef NewPacket<8192> http_packet;
   // typedef NewPacket<1024> http_packet;
  // typedef NewPacket<512> http_packet;
  // typedef NewPacket<256> http_packet;
     // typedef NewPacket<128> http_packet;
  // typedef NewPacket<64> http_packet;
   struct websocket_header
   {
      //  this header is the first 16 bits.
      //  0                   1                   2                   3
      //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      // +-+-+-+-+-------+-+-------------+-------------------------------+
      // |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
      // |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
      // |N|V|V|V|       |S|             |   (if payload len==126/127)   |
      // | |1|2|3|       |K|             |                               |
      // +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
      websocket_header() : opcode(1), RSV3(0), RSV2(0), RSV1(0), FIN(1), length(0), MASK(0)
      {
	 // consider c++20 for int a : 7 = 1; syntax
      }
      uint8_t opcode : 4;
      uint8_t RSV3 : 1;
      uint8_t RSV2 : 1;
      uint8_t RSV1 : 1;
      uint8_t FIN  : 1;
      uint8_t length : 7;
      uint8_t MASK : 1;
   };
   const char ws_handshake[] =
   {
      "HTTP/1.1 101 Switching Protocols\r\n"
      "Upgrade: websocket\r\n"
      "Connection: Upgrade\r\n"
      "Sec-WebSocket-Accept: "
   };
}

bool send_file(const net::address& lv_address,  net::socket& lv_socket,  net::http_packet& lv_packet, uint32_t start, uint32_t end);
int write_response_header(net::http_packet& lv_packet, std::string_view request, int& start, int& end);

void net::http_server::mf_server_thread(const socket& in_socket)
{
   auto topic = log::scope("http_server");
   net::socket lv_listen_socket(in_socket);
   //Initialize some things
   net::http_packet lv_packet;
   auto lf_html_404 =
      [&lv_packet]
      {
	 lv_packet.Clear();
	 char lv_html_404[] =
	    {
	       "HTTP/1.1 404 resource not found\r\n"
	       "\r\n"
	    };
	 lv_packet.IterWrite<decltype(lv_html_404)>(lv_html_404);
      };
   auto lf_html_200 =
      [&lv_packet]()
      {
	 lv_packet.Clear();
	 lv_packet.IterWrite("HTTP/1.1 200 OK\r\n");
	 lv_packet.IterWrite("Connection: Keep-Alive");
	 lv_packet.IterWrite( "\r\n\r\n");
      };

   net::address lv_address;
   net::socket lv_socket;
   log::debug("----------------");
   log::debug("starting http server");

   bool print_waiting = true;
   while(mv_running)
   {
      if(print_waiting)
      {
	 log::debug("----------------");
	 log::debug("waiting to accept...");
	 print_waiting = false;
      }
      if(lv_listen_socket.Accept(lv_address, lv_socket))
      {
	 print_waiting = true;
	 log::debug("accepted: ");
	 lv_address.PrintDetails();
	 bool lv_close_socket = true;
	 float lv_timeout = 0.5;
	 float lv_last_timeout = 0;
	 std::chrono::high_resolution_clock lv_clock;
	 auto lv_start_time = lv_clock.now();
	 do
	 {
	    lv_packet.Clear();
	    int lv_bytes_read = lv_socket.receive(lv_packet.GetData(), lv_packet.GetCapacity());
	    if (lv_bytes_read > 0)
	    {
	       lv_packet.SetLength(lv_bytes_read);
	       lv_packet.PrintDetails();
	       // todo: this should be a string_view but emacs wont stop complaining.
	       std::string_view view((char*)lv_packet.GetData());
	       auto http_loc = view.find("HTTP/");
	       if (view.find("Connection: keep-alive") != std::string::npos || view.find("Connection: Keep-Alive") != std::string::npos)
	       {
		  lv_timeout = 0.5;
		  lv_start_time = lv_clock.now();
	       }
	       if (http_loc != std::string::npos)
	       {
		  auto get_loc = view.find("GET /");
		  if(get_loc != std::string::npos)
		  {
		     lv_packet.Clear();
		     // todo: fix this
		     std::string get_request(view.substr(get_loc + 5, http_loc - 6));
		     log::debug("get: /%s", get_request.c_str());
		     //
		     if (get_request.compare("ws") == 0)
		     {
			//
			// Handle WS upgrade request
			//

			char key_start_str[] = {"Sec-WebSocket-Key: "};
			auto key_start_loc = view.find(key_start_str) + sizeof(key_start_str) - 1;
			//
			char key_end_str[] = {"==\r\n"};
			auto key_end_loc = (view.find(key_end_str) + 2) - key_start_loc;
			//
			unsigned char hash[28];
			memset(&hash[0], 0, sizeof(hash));
			// The stuff here is pretty cryptic sadly,
			// Generating a sha key and encoding it into
			// base64 hash.
			{
			   // todo: generate different GUIDs?
			   const char GUID[] = {"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};
			   std::string key(view.substr(key_start_loc, key_end_loc));
			   key += GUID;
			   unsigned char sha[20];
			   net::encrypt::SHA1(sha, (unsigned char *)key.data(), key.length());
			   net::encode::Base64(&hash[0], &sha[0], sizeof(sha));
			}
			lv_packet.IterWrite(net::ws_handshake);
			lv_packet.IterWrite(hash);
			unsigned char packet_end[] = {"\r\n\r\n"};
			lv_packet.IterWrite(packet_end);
			//
			log::debug("sending: websocket upgrade");
			lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
			lv_packet.Clear();
		  
			// starting ws thread.
			lv_close_socket = false;
			lv_timeout = 0;
			lv_socket.mf_set_keepalive(true);
			mv_ws_threads.push_back(new std::thread(&net::http_server::mf_ws_thread, this, lv_socket, lv_address));
			// sleep here because those objects are passed by ref and I don't want them to be destroyed.
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		     }
		     else
		     {
			//
			// Handle regular http get requests.
			//
			const char *lv_request = get_request.length() == 0 ? "index.html" : get_request.c_str();
			std::string lv_request_view(lv_request);
			bool lv_handled = false;
			if (mv_handler != nullptr)
			{
			   lf_html_200(); 
			   auto lv_get_response =
			      [&lv_handled, &lv_packet]
			      (const char *message, size_t size)
			      {
				 lv_handled = true;
				 lv_packet.IterWrite(message, size);
			      };
			   mv_handler->mf_get_response(lv_request, lv_get_response);
			}
			if (lv_handled != true)
			{
			   std::string lv_file_path;
			   if (mv_handler != nullptr)
			   {
			      lv_file_path.append(mv_handler->mf_get_response_root_dir());
			   }
			   lv_file_path.append(lv_request);
			   if(lv_file_path.find("../") != std::string::npos)
			   {
			      // todo: write access denied
			      // not allowing ../, illegal access pattern.
			      lf_html_404();
			   }
			   else
			   {
			      lv_packet.OpenFile(lv_file_path.c_str());
			      if(lv_packet.IsFileOpen())
			      {
				 int start = 0, end = 0;
				 auto lv_content_length = std::to_string(lv_packet.GetFileSize());
				 int lv_response_code = write_response_header(lv_packet, view, start, end);
				 if(lv_response_code == 200 || lv_response_code == 206)
				 {

				    log::debug("sending %s", lv_file_path.c_str());
				    if(send_file(lv_address, lv_socket, lv_packet, start, end))
				    {
				       if(start == 0 && end == 0)
				       {
					  //todo: this assumes about a 500kbs download speed.
					  //      try to drive this via some metric rather than
					  //      with a static magic number.
					  lv_timeout = ((float)(lv_packet.GetFileSize()))/500000.0;
				       }
				       else
				       {
					  lv_timeout = ((float)(end - start))/(500000.0);
				       }
				       lv_timeout += 1.0f/30.0f;
				    }
				    else
				    {
				       lv_timeout = 0;
				    }
				 }
				 lv_start_time = lv_clock.now();
			      }
			      else
			      {
				 lf_html_404();
			      }
			      lv_packet.CloseFile();
			   }
			}
			lv_packet.PrintDetails();
		     }
		     if(lv_packet.GetSize() > 0)
		     {
			lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
			lv_packet.Clear();
		     }
		     log::debug("----------------");
		  }
		  else
		  {
		     auto post_loc = view.find("POST /");
		     if(post_loc != std::string::npos)
		     {
			std::string post_request(view.substr(post_loc + 5, http_loc - 6));
			log::debug("post request: %.*s", post_request.length(), post_request.data());
			if(std::ends_with(post_request, "/params"))
			{
			   //todo: this can be a string view
			   std::string_view body((char*)lv_packet.GetData(), lv_packet.GetSize());
			   auto body_pos = body.find("\r\n\r\n");
			   if(body_pos != std::string::npos && (body.length()-body_pos) > 4)
			   {
			      log::debug("parsing params post body");
			      commandline::parse((char*)body.substr(body_pos+4).data());
			   }
			   else
			   {
			      lv_timeout = 1;
			      lv_start_time = lv_clock.now();
			      do
			      {
				 lv_packet.Clear();
				 std::this_thread::sleep_for(std::chrono::milliseconds(30));
				 lv_bytes_read = lv_socket.receive(lv_packet.GetData(), lv_packet.GetCapacity());
			      } while(lv_timeout > std::chrono::duration<float>(lv_clock.now()-lv_start_time).count() && lv_bytes_read == 0);
			      lv_packet.SetLength(lv_bytes_read);
			      if(lv_bytes_read > 0)
			      {
				 log::debug("parsing params post");
				 commandline::parse((char*)lv_packet.GetData());  
			      }
			      else
			      {
				 log::debug("timed out waiting params for post body");
			      }
			   }
			}
		     }
		  }
	       }  
	    }
	    if(lv_timeout > lv_last_timeout)
	    {
	       float lv_until = lv_timeout - std::chrono::duration<float>(lv_clock.now()-lv_start_time).count();
	       log::debug("timeout in: %f secs", lv_until);
	       lv_last_timeout = lv_timeout;
	    }
	 }while(lv_timeout > std::chrono::duration<float>(lv_clock.now()-lv_start_time).count());
	 log::debug("connection timedout: %f secs", std::chrono::duration<float>(lv_clock.now()-lv_start_time).count());
	 if(lv_close_socket)
	 {
	    lv_socket.closeSock();
	 }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   log::debug("ending http server");
}

void net::http_server::mf_ws_thread(const net::socket& from, const net::address& to)
{
   auto topic = log::scope("websocket");
   // This thread is pointless if there is no
   // handler set.
   net::socket lv_socket(from);
   net::address lv_address(to);
   net::http_packet lv_packet;
   log::debug("begin websocket thread");
   lv_socket.mf_set_nonblocking(true);
   websocket_header ws_header;
   bool lv_is_connected = true;
   while (mv_running && lv_is_connected)
   {
      // todo: this might not be thread safe, I'm not sure.
      auto lv_ws_send =
	 [&lv_packet, &ws_header, &lv_socket, &lv_address, &lv_is_connected]
	 (const char *message,size_t size)
	 {
	    // todo: make sure the correct message size is written here.
	    lv_packet.Clear();
	    ws_header.length = size - 1;
	    lv_packet.IterWrite(ws_header);
	    lv_packet.IterWrite(message, size);
	    if(lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize()) == -1)
	    {
	       log::debug("websocket connection broken");
	       lv_is_connected = false;
	    };
	 };
      // spin lock waiting for messages.
      lv_packet.Clear();
      int lv_bytes_read = lv_socket.receive(lv_packet.GetData(), lv_packet.GetCapacity());
      if (lv_bytes_read > 0)
      {
	 log::debug("------");
	 auto in_header = lv_packet.IterRead<websocket_header>();
	 unsigned char mask[4];
	 unsigned char data[in_header.length + 1];
	 data[in_header.length] = '\0';
	 lv_packet.IterRead(mask[0], 4);
	 lv_packet.IterRead(data[0], in_header.length);
	 log::debug("client_sends: ");
	 for (unsigned int i = 0; i < in_header.length; ++i)
	 {
	    // data[i] ^= mask[i%4]; below is equvivilant
	    // think [i & 3] is faster.
	    data[i] ^= mask[i & 3];
	 }
	 log::debug("%s", data);
	 if (mv_handler != nullptr)
	 {
	    mv_handler->mf_ws_response((const char *)&data[0], lv_ws_send);
	 }
	 log::debug("------");
      }
      if (mv_handler != nullptr)
      {
	 mv_handler->mf_ws_send(lv_ws_send);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   lv_socket.closeSock();
}

net::http_server::~http_server()
{
   //shutdown all the threads
   mv_running = false;
   if(mv_server_thread != nullptr)
   {
      mv_server_thread->join();
      delete mv_server_thread;
   }
   for(auto* ws_thread : mv_ws_threads)
   {
      ws_thread->join();
      delete ws_thread;
   }
}
// file handlers.
bool send_file(const net::address& lv_address,  net::socket& lv_socket,  net::http_packet& lv_packet, std::uint32_t start = 0, std::uint32_t end = 0)
{
   if(lv_packet.IsFileOpen())
   {
      if(end == 0 || end > lv_packet.GetFileSize())
      {
	 end = lv_packet.GetFileSize();
      }
      int bytes_to_write = end - start;
      int bytes_written = 0;
      do
      {
	 int space = lv_packet.GetRemainingSpace(); //reserving 8 bytes
	 int chunk_size = space > bytes_to_write ? bytes_to_write : space;
	 bytes_written += lv_packet.WriteFile(start+bytes_written, end, chunk_size);
	 if(lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize()) == -1)
	 {
	    log::debug("failed to send file.");
	    lv_packet.Clear();
	    return false;
	 }
	 lv_packet.Clear();
	 std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
      while(bytes_written < bytes_to_write);
      log::debug("sent %d", bytes_written);
      return true;
   }
   log::debug("file not open...");
   return false;
}
int write_response_header(net::http_packet& lv_packet, std::string_view request, int& start, int& end)
{
   if(lv_packet.IsFileOpen())
   {
      lv_packet.Clear();
      std::string_view lv_file_name(lv_packet.GetFileName());
      std::string_view lv_content_type;
      if(lv_file_name.find(".html") != std::string::npos)
      {
	 lv_content_type = "Content-Type: text/html\r\n";
      }
      else if(lv_file_name.find(".js") != std::string::npos)
      {
	 lv_content_type = "Content-Type: text/javascript\r\n";
      }
      else if (lv_file_name.find(".css") != std::string::npos)
      {
	 lv_content_type = "Content-Type: text/css;\r\n";
      }
      else if (lv_file_name.find(".mp4") != std::string::npos)
      {
	 lv_content_type = "Content-Type: video/mp4\r\n";
      }
      
      std::string lv_file_size = std::to_string(lv_packet.GetFileSize());
      char lv_range_str[] = { "Range: bytes=" };
      auto partial = request.find(lv_range_str);
      if(partial != std::string::npos)
      {
	 auto lv_end_offset = request.find("\r\n", partial)-(partial+sizeof(lv_range_str)-1);

	 std::string lv_partial_range(request.substr(partial+sizeof(lv_range_str)-1, lv_end_offset));
	 int lv_num_left = 0;
	 int lv_num_right = 0;

	 auto lv_number_split = lv_partial_range.find("-");
	 lv_num_left = std::from_string<int>(lv_partial_range.substr(0, lv_number_split));
	 auto lv_num_right_str = lv_partial_range.substr(lv_number_split+1, lv_partial_range.npos);
	 if (!lv_num_right_str.empty())
	 {
	    lv_num_right = std::from_string<int>(lv_num_right_str);	       
	 }
	 else
	 {
	    lv_num_right = std::from_string<int>(lv_file_size)-1;
	    lv_partial_range.append(std::to_string(lv_num_right));
	 }

	 int partial_bytes = (lv_num_right - lv_num_left)+1;
	 std::string lv_content_length = std::to_string(partial_bytes);
	 if(partial_bytes > 0)
	 {
	    lv_packet.IterWrite("HTTP/1.1 206 Partial Content\r\n");
	    lv_packet.IterWrite(lv_content_type.data(), lv_content_type.length());
	    lv_packet.IterWrite("Content-Length: ");
	    lv_packet.IterWrite(lv_content_length.data(), lv_content_length.length());
	    lv_packet.IterWrite("\r\n");
	    lv_packet.IterWrite("Connection: Keep-Alive\r\n");
	    lv_packet.IterWrite("Accept-Ranges: bytes\r\n");
	    lv_packet.IterWrite("Content-Range: bytes ");
	    lv_packet.IterWrite(lv_partial_range.c_str(), lv_partial_range.length());
	    lv_packet.IterWrite(" / ");
	    lv_packet.IterWrite(lv_file_size.c_str(), lv_file_size.length());
	    lv_packet.IterWrite("\r\n\r\n");
	    start = lv_num_left;
	    //byte ranges are inclusive! 0-1 = 2bytes
	    end = lv_num_right+1;
	    lv_packet.PrintDetails();
	    return 206;
	 }
	 else
	 {
	    lv_packet.IterWrite("HTTP/1.1 416 Range Not Satisfiable\r\n");
	    lv_packet.IterWrite(lv_content_type.data(), lv_content_type.length());
	    lv_packet.IterWrite("Connection: Keep-Alive\r\n");
	    lv_packet.IterWrite("Accept-Ranges: bytes\r\n");
	    lv_packet.IterWrite("Content-Length: ");
	    lv_packet.IterWrite(lv_file_size.data(), lv_file_size.length());
	    lv_packet.IterWrite("\r\n\r\n");
	    lv_packet.PrintDetails();
	    return 416;
	 }
      }
      else
      {
	 lv_packet.IterWrite("HTTP/1.1 200 OK\r\n");
	 lv_packet.IterWrite(lv_content_type.data(), lv_content_type.length());
	 lv_packet.IterWrite("Connection: Keep-Alive\r\n");
	 lv_packet.IterWrite("Accept-Ranges: bytes\r\n");
	 lv_packet.IterWrite("Content-Length: ");
	 lv_packet.IterWrite(lv_file_size.data(), lv_file_size.length());
	 lv_packet.IterWrite("\r\n\r\n");
	 lv_packet.PrintDetails();
	 return 200;
      }
   }
   return false;
}




