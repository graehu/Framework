#include "http_server.h"
#include "../packet/packet.h"
#include "address.h"
#include "socket.h"
#include "../utils/encrypt.h"
#include "../utils/encode.h"
// lib includes
#include <bits/stdint-uintn.h>
#include <cstddef>
#include <cstdio>
#include <ios>
#include <memory>
#include <netinet/in.h>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>

#define show_val(variable) printf(#variable": %d\n", variable);
#define if_logging(expression) if(mv_logging) { expression; }
net::http_server::http_server(unsigned int port) :
   mv_handler(nullptr),
   mv_server_thread(nullptr),
   mv_logging(false),
   mv_running(true)
{
   net::socket lv_listen_socket(net::socket::eHttpSocket);
   for(int i = 0; !lv_listen_socket.openSock(port+i) && i < 100; i++) { }
   //todo: find out exactly how unsafe this is.
   lv_listen_socket.mf_set_keepalive(true);
   mv_server_thread = new std::thread(&net::http_server::mf_server_thread, this, lv_listen_socket);
}
namespace net
{
   typedef NewPacket<8192> http_packet;
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

void net::http_server::mf_server_thread(const socket& in_socket)
{
   net::socket lv_listen_socket(in_socket);
   // net::socket listen_socketket(*mv_listen_socket);
   //Initialize some things
   net::http_packet lv_packet;
   auto lf_html_404 =
      [&lv_packet]
      {
	 lv_packet.Clear();
	 char lv_html_404[] =
	    {
	       "HTTP/1.0 404 resource not found\r\n"
	       "\r\n"
	    };
	 lv_packet.IterWrite<decltype(lv_html_404)>(lv_html_404);
      };
   enum content_types
   {
    e_none,
    e_html,
    e_javascript,
   };
   auto lf_html_200 =
     [&lv_packet](content_types type = e_none)
      {
	 lv_packet.Clear();
	 char lv_html_200[] = { "HTTP/1.0 200 OK\r\n" };
	 lv_packet.IterWrite(lv_html_200);
	 if(type != e_none)
	   {
	      char lv_content_type[] = { "Content-Type: " };
	      lv_packet.IterWrite(lv_content_type);
	      switch(type)
	      {
		 case e_html:
		 {
		    char lv_type[] = { "text/html;" };
		    lv_packet.IterWrite(lv_type);
		 }
		 break;
		 case e_javascript:
		 {
		    char lv_type[] = { "text/javascript;" };
		    lv_packet.IterWrite(lv_type);
		 }
		 break;
		 case e_none:
		 default:
		    break;
	      } 
	     char lv_ending[] = { "\r\n\r\n" };
	     lv_packet.IterWrite(lv_ending);
	   }
      };

   net::address lv_address;
   net::socket lv_socket;
   if(mv_logging)
   {
      printf("starting http server\n");
   }
   while(mv_running)
   {
      if(lv_listen_socket.Accept(lv_address, lv_socket))
      {
	 lv_packet.Clear();
	 int lv_bytes_read = lv_socket.receive(lv_packet.GetData(), lv_packet.GetCapacity());
	 if (lv_bytes_read > 0)
	 {
	    bool lv_close_socket = true;
	    if (mv_logging)
	    {
	       printf("----------------\n");
	       printf("accepted: ");
	       lv_address.PrintDetails();
	       printf("\n");
	    }
	    lv_packet.SetLength(lv_bytes_read);
	    // todo: this should be a string_view but emacs wont stop complaining.
	    std::string view((char*)lv_packet.GetData());
	    auto http_loc = view.find("HTTP/");
	    auto get_loc = view.find("GET /");

	    if (http_loc != std::string::npos && get_loc != std::string::npos)
	    {
	       lv_packet.Clear();
	       // todo: fix this
	       std::string get_request = view.substr(get_loc + 5, http_loc - 6);
	       if (mv_logging)
	       {
		  printf("get: /%s\n", get_request.c_str());
	       }
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
		     std::string key = view.substr(key_start_loc, key_end_loc) + GUID;
		     unsigned char sha[20];
		     net::encrypt::SHA1(sha, (unsigned char *)key.data(), key.length());
		     net::encode::Base64(&hash[0], &sha[0], sizeof(sha));
		  }
		  lv_packet.IterWrite(net::ws_handshake);
		  lv_packet.IterWrite(hash);
		  unsigned char packet_end[] = {"\r\n\r\n"};
		  lv_packet.IterWrite(packet_end);
		  //
		  if (mv_logging)
		  {
		     printf("sending: websocket upgrade\n");
		  }
		  lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
		  lv_packet.Clear();
		  
		  // starting ws thread.
		  lv_close_socket = false;
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
		  if(lv_request_view.find(".js") != std::string::npos)
		  {
		     lf_html_200(e_javascript);
		  }
		  else if(lv_request_view.find(".html") != std::string::npos)
		  {
		     lf_html_200(e_html);
		  }
		  else
		  {
		     lf_html_200();		     
		  }

		  bool lv_handled = false;
		  if (mv_handler != nullptr)
		  {
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
			auto file_status = lv_packet.WriteFile(lv_file_path.c_str());
			if(file_status == BasePacket::e_complete)
			{
			   if (mv_logging)
			   {
			      printf("sending: 200, %s\n", lv_request);
			   }  
			}
			else if(file_status == BasePacket::e_in_progress)
			{
			   //continuous send if files are bigger than the packets size.
			   do
			   {
			      if(mv_logging)
			      {
				 lv_packet.PrintDetails();
			      }
			      lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
			      lv_packet.Clear();
			      file_status = lv_packet.WriteFile(lv_file_path.c_str());
			      
			   }
			   while(file_status == BasePacket::e_in_progress);
			   
			   if (mv_logging)
			   {
			      printf("sending: 200, %s\n", lv_request);
			   }
			}
			else if(file_status == BasePacket::e_failed)
			{
			   lf_html_404();
			   if (mv_logging)
			   {
			      printf("sending: 404, %s not found\n", lv_request);
			   }
			}
		     }
		  }
		  if(mv_logging)
		  {
		     lv_packet.PrintDetails();   
		  }
		  lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
	       }
	       if (mv_logging)
	       {
		  printf("----------------\n");
	       }
	    }
	    if(lv_close_socket)
	    {
	       lv_socket.closeSock();
	    }
	 }
	 else
	 {
	    if(mv_logging)
	    {
	       printf("failed to recieve\n");
	    }
	 }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   if(mv_logging)
   {
      printf("ending http server\n");      
   }
}

void net::http_server::mf_ws_thread(const net::socket& from, const net::address& to)
{
   // This thread is pointless if there is no
   // handler set.
   net::socket lv_socket(from);
   net::address lv_address(to);
   net::http_packet lv_packet;
   if (mv_logging)
   {
      printf("begin websocket thread\n");
   }
   lv_socket.mf_set_nonblocking(true);
   websocket_header ws_header;
   while (mv_running)
   {
      auto lv_ws_send =
	 [this, &lv_packet, &ws_header, &lv_socket, &lv_address]
	 (const char *message,size_t size)
	 {
	    // todo: make sure the correct message size is written here.
	    lv_packet.Clear();
	    ws_header.length = size - 1;
	    lv_packet.IterWrite(ws_header);
	    lv_packet.IterWrite(message, size);
	    if(mv_logging)
	    {
	       lv_packet.PrintDetails();   
	    }
	    lv_socket.send(lv_address, lv_packet.GetData(), lv_packet.GetSize());
	 };
      // spin lock waiting for messages.
      lv_packet.Clear();
      int lv_bytes_read = lv_socket.receive(lv_packet.GetData(), lv_packet.GetCapacity());
      if (lv_bytes_read > 0)
      {
	 if (mv_logging)
	 {
	    printf("------\n");
	 }
	 auto in_header = lv_packet.IterRead<websocket_header>();
	 unsigned char mask[4];
	 unsigned char data[in_header.length + 1];
	 data[in_header.length] = '\0';
	 lv_packet.IterRead(mask[0], 4);
	 lv_packet.IterRead(data[0], in_header.length);
	 if (mv_logging)
	 {
	    printf("client_sends: \n");
	 }
	 for (unsigned int i = 0; i < in_header.length; ++i)
	 {
	    // data[i] ^= mask[i%4]; below is equvivilant
	    // think [i & 3] is faster.
	    data[i] ^= mask[i & 3];
	    if (mv_logging)
	    {
	       printf("%c", data[i]);
	    }
	 }
	 if (mv_logging)
	 {
	    printf("\n");   
	 }
	 if (mv_handler != nullptr)
	 {
	    mv_handler->mf_ws_response((const char *)&data[0], lv_ws_send);
	 }
	 if (mv_logging)
	 {
	    printf("\n");
	    printf("------\n");
	 }
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
