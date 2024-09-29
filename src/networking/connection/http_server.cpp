#include "http_server.h"
#include "../packet/packet.h"
#include "address.h"
#include "socket.h"
#include "../utils/encrypt.h"
#include "../utils/encode.h"
#include "../../utils/params.h"
#include "../../utils/string_helpers.h"
#include "../../utils/log/log.h"
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
#include <sstream>
#include <thread>
#include <chrono>
#include <type_traits>

using namespace fw;

void fw::commandline::http_handler::post_response(std::string_view& request, std::string_view& body)
{
   if(std::ends_with(request, "/params"))
   {
      commandline::parse((char*)body.data());
   }
}

net::http_server::http_server(unsigned int port) :
   m_root_dir(""),
   m_server_thread(nullptr),
   m_running(true)
   
{
   static bool do_once = true;
   if(do_once)
   {
      log::topics::add("websocket");
      log::topics::add("http_server");
      do_once = false;
   }
      
   net::socket listen_socket(net::socket::eHttpSocket);
   for(int i = 0; !listen_socket.openSock(port+i) && i < 100; i++) { }
   // #todo: find out exactly how unsafe this is.
   listen_socket.set_keepalive(true);
   m_server_thread = new std::thread(&net::http_server::server_thread, this, listen_socket);   
}
namespace net
{
   // #todo: test a smaller packet.
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

bool send_file(const net::address& address,  net::socket& socket,  net::http_packet& packet, uint32_t start, uint32_t end);
int write_response_header(net::http_packet& packet, std::string_view request, int& start, int& end);

void net::http_server::server_thread(const socket& in_socket)
{
   log::scope topic("http_server");
   net::socket listen_socket(in_socket);
   //Initialize some things
   net::http_packet packet;
   auto html_404 =
      [&packet]
      {
	 packet.Clear();
	 char html_404[] =
	    {
	       "HTTP/1.1 404 resource not found\r\n"
	       "\r\n"
	    };
	 packet.IterWrite<decltype(html_404)>(html_404);
      };
   auto html_200 =
      [&packet]()
      {
	 packet.Clear();
	 packet.IterWrite("HTTP/1.1 200 OK\r\n");
	 packet.IterWrite("Connection: Keep-Alive");
	 packet.IterWrite( "\r\n\r\n");
      };

   net::address address;
   net::socket socket;
   log::debug("----------------");
   log::debug("starting http server");

   bool print_waiting = true;
   while(m_running)
   {
      if(print_waiting)
      {
	 log::debug("----------------");
	 log::debug("waiting to accept...");
	 print_waiting = false;
      }
      if(listen_socket.Accept(address, socket))
      {
	 print_waiting = true;
	 log::debug("accepted: ");
	 address.PrintDetails();
	 bool close_socket = true;
	 float timeout = 0.5;
	 float last_timeout = 0;
	 std::chrono::high_resolution_clock clock;
	 auto start_time = clock.now();
	 do
	 {
	    packet.Clear();
	    int bytes_read = socket.receive(packet.GetData(), packet.GetCapacity());
	    if (bytes_read > 0)
	    {
	       packet.SetLength(bytes_read);
	       packet.PrintDetails();
	       std::string_view view((char*)packet.GetData());
	       auto http_loc = view.find("HTTP/");
	       if (view.find("Connection: keep-alive") != std::string::npos || view.find("Connection: Keep-Alive") != std::string::npos)
	       {
		  timeout = 0.5;
		  start_time = clock.now();
	       }
	       if (http_loc != std::string::npos)
	       {
		  auto get_loc = view.find("GET /");
		  if(get_loc != std::string::npos)
		  {
		     packet.Clear();
		     //  #note: these aren't magic numbers, but it could be clearer
		     //         +5 is len of GET / and the start of http_loc is always 6 chars longer
		     //         than the request.
		     std::string get_request(view.substr(get_loc + 5, http_loc - 6));
		     log::debug("get: /{}", get_request.c_str());
		     //
		     if (get_request.compare("ws") == 0)
		     {
			//  #todo: Move this to a custom handler
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
			   //  #todo: generate different GUIDs?
			   const char GUID[] = {"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};
			   std::string key(view.substr(key_start_loc, key_end_loc));
			   key += GUID;
			   unsigned char sha[20];
			   net::encrypt::SHA1(sha, (unsigned char *)key.data(), key.length());
			   net::encode::Base64(&hash[0], &sha[0], sizeof(sha));
			}
			packet.IterWrite(net::ws_handshake);
			packet.IterWrite(hash);
			unsigned char packet_end[] = {"\r\n\r\n"};
			packet.IterWrite(packet_end);
			//
			log::debug("sending: websocket upgrade");
			socket.send(address, packet.GetData(), packet.GetSize());
			packet.Clear();
		  
			// starting ws thread.
			close_socket = false;
			timeout = 0;
			socket.set_keepalive(true);
			m_ws_threads.push_back(new std::thread(&net::http_server::ws_thread, this, socket, address));
			// sleep here because those objects are passed by ref and I don't want them to be destroyed.
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		     }
		     else
		     {
			//
			// Handle regular http get requests.
			//
			const char *request = get_request.length() == 0 ? "index.html" : get_request.c_str();
			std::string request_view(request);
			bool handled = false;
			if (!m_handlers.empty())
			{
			   for(auto* handler : m_handlers)
			   {
			      html_200(); 
			      auto get_response =
				 [&handled, &packet]
				 (const char *message, size_t size)
				 {
				    handled = true;
				    packet.IterWrite(message, size);
				 };
			      handler->get_response(request, get_response);
			      if(handled)
			      {
				 break;
			      }
			   }
			}
			if (handled != true)
			{
			   //  #todo: move this not handled code into a func, if possible
			   std::string file_path(m_root_dir);
			   file_path.append(request);
			   if(file_path.find("../") != std::string::npos)
			   {
			      //  #todo: return access denied code.
			      // not allowing ../, illegal access pattern.
			      html_404();
			   }
			   else
			   {
			      packet.OpenFile(file_path.c_str());
			      if(packet.IsFileOpen())
			      {
				 int start = 0, end = 0;
				 auto content_length = std::to_string(packet.GetFileSize());
				 int response_code = write_response_header(packet, view, start, end);
				 if(response_code == 200 || response_code == 206)
				 {

				    log::debug("sending {}", file_path.c_str());
				    if(send_file(address, socket, packet, start, end))
				    {
				       if(start == 0 && end == 0)
				       {
					  // #todo: this assumes about a 500kbs download speed.
					  //      try to drive this via some metric rather than
					  //      with a static magic number.
					  timeout = ((float)(packet.GetFileSize()))/500000.0;
				       }
				       else
				       {
					  timeout = ((float)(end - start))/(500000.0);
				       }
				       timeout += 1.0f/30.0f;
				    }
				    else
				    {
				       timeout = 0;
				    }
				 }
				 start_time = clock.now();
			      }
			      else
			      {
				 html_404();
			      }
			      packet.CloseFile();
			   }
			}
			packet.PrintDetails();
		     }
		     if(packet.GetSize() > 0)
		     {
			socket.send(address, packet.GetData(), packet.GetSize());
			packet.Clear();
		     }
		     log::debug("----------------");
		  }
		  else
		  {
		     auto post_loc = view.find("POST /");
		     if(post_loc != std::string::npos)
		     {
			std::string post_request(view.substr(post_loc + 5, http_loc - 6));
			std::string_view post_request_view(post_request);
			log::debug("post request: {:.{}}", post_request.data(), post_request.length());
			std::string_view body((char*)packet.GetData(), packet.GetSize());
			auto body_pos = body.find("\r\n\r\n");
			//if(std::ends_with(post_request, "/params"))
			if(!m_handlers.empty())
			{
			   if(body_pos != std::string::npos && (body.length()-body_pos) > 4)
			   {
			      log::debug("passing post request to handlers");
			      std::string_view body_view(body.substr(body_pos+4));
			      for(auto* handler : m_handlers)
			      {
				 handler->post_response(post_request_view, body_view);
			      }
			      // commandline::parse((char*)body.substr(body_pos+4).data());
			   }
			   else
			   {
			      timeout = 1;
			      start_time = clock.now();
			      do
			      {
				 packet.Clear();
				 std::this_thread::sleep_for(std::chrono::milliseconds(30));
				 bytes_read = socket.receive(packet.GetData(), packet.GetCapacity());
			      } while(timeout > std::chrono::duration<float>(clock.now()-start_time).count() && bytes_read == 0);
			      packet.SetLength(bytes_read);
			      if(bytes_read > 0)
			      {
				 log::debug("parsing params post");
				 std::string_view body_view((char*)packet.GetData(), bytes_read);
				 for(auto* handler : m_handlers)
				 {
				    handler->post_response(post_request_view, body_view);
				 }
				 // commandline::parse((char*)packet.GetData());  
			      }
			      else
			      {
				 log::debug("timed out waiting params for post body");
				 std::string_view body_view("");
				 for(auto* handler : m_handlers)
				 {
				    handler->post_response(post_request_view, body_view);
				 }
			      }
			   }
			}
		     }
		  }
	       }  
	    }
	    if(timeout > last_timeout)
	    {
	       float until = timeout - std::chrono::duration<float>(clock.now()-start_time).count();
	       log::debug("timeout in: {} secs", until);
	       last_timeout = timeout;
	    }
	 }while(timeout > std::chrono::duration<float>(clock.now()-start_time).count());
	 log::debug("connection timedout: {} secs", std::chrono::duration<float>(clock.now()-start_time).count());
	 if(close_socket)
	 {
	    socket.closeSock();
	 }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   log::debug("ending http server");
}

void net::http_server::ws_thread(const net::socket& from, const net::address& to)
{
   log::scope topic("websocket");
   // This thread is pointless if there is no
   // handler set.
   net::socket socket(from);
   net::address address(to);
   net::http_packet packet;
   log::debug("begin websocket thread");
   socket.set_nonblocking(true);
   websocket_header ws_header;
   bool is_connected = true;
   handler* ws_handler = nullptr;
   for(auto* handler : m_handlers)
   {
      if(handler->is_ws_handler())
      {
	 ws_handler = handler;
	 break;
      }
   }
   while (m_running && is_connected)
   {
      //  #todo: test if this is thread safe
      auto ws_send =
	 [&packet, &ws_header, &socket, &address, &is_connected]
	 (const char *message,size_t size)
	 {
	    //  #todo: split packets that are too big for websocket, or bail
	    packet.Clear();
	    ws_header.length = size - 1;
	    packet.IterWrite(ws_header);
	    packet.IterWrite(message, size);
	    if(socket.send(address, packet.GetData(), packet.GetSize()) == -1)
	    {
	       log::debug("websocket connection broken");
	       is_connected = false;
	    };
	 };
      // spin lock waiting for messages.
      packet.Clear();
      int bytes_read = socket.receive(packet.GetData(), packet.GetCapacity());
      // #todo: this only sends if we're recieving, that's not correct.
      if (bytes_read > 0)
      {
	 log::debug("------");
	 auto in_header = packet.IterRead<websocket_header>();
	 unsigned char mask[4];
	 unsigned char data[sizeof(websocket_header) + 1];
	 data[in_header.length] = '\0';
	 packet.IterRead(mask[0], 4);
	 packet.IterRead(data[0], in_header.length);
	 log::debug("client_sends: ");
	 for (unsigned int i = 0; i < in_header.length; ++i)
	 {
	    // data[i] ^= mask[i%4]; below is equvivilant
	    // think [i & 3] is faster.
	    data[i] ^= mask[i & 3];
	 }
	 // #todo: test this debug.
	 log::debug("{}", (const char*)&data);
	 if (ws_handler != nullptr)
	 {
	    ws_handler->ws_response((const char *)&data[0], ws_send);
	 }
	 log::debug("------");
      }
      if (ws_handler != nullptr)
      {
	 ws_handler->ws_send(ws_send);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(30));
   }
   socket.closeSock();
}

net::http_server::~http_server()
{
   //shutdown all the threads
   m_running = false;
   if(m_server_thread != nullptr)
   {
      m_server_thread->join();
      delete m_server_thread;
   }
   for(auto* ws_thread : m_ws_threads)
   {
      ws_thread->join();
      delete ws_thread;
   }
}
// file handlers.
bool send_file(const net::address& address,  net::socket& socket,  net::http_packet& packet, std::uint32_t start = 0, std::uint32_t end = 0)
{
   if(packet.IsFileOpen())
   {
      if(end == 0 || end > packet.GetFileSize())
      {
	 end = packet.GetFileSize();
      }
      int bytes_to_write = end - start;
      int bytes_written = 0;
      do
      {
	 int space = packet.GetRemainingSpace(); //reserving 8 bytes
	 int chunk_size = space > bytes_to_write ? bytes_to_write : space;
	 bytes_written += packet.WriteFile(start+bytes_written, end, chunk_size);
	 if(socket.send(address, packet.GetData(), packet.GetSize()) == -1)
	 {
	    log::debug("failed to send file.");
	    packet.Clear();
	    return false;
	 }
	 packet.Clear();
	 std::this_thread::sleep_for(std::chrono::milliseconds(30));
      }
      while(bytes_written < bytes_to_write);
      log::debug("sent {}", bytes_written);
      return true;
   }
   log::debug("file not open...");
   return false;
}
int write_response_header(net::http_packet& packet, std::string_view request, int& start, int& end)
{
   if(packet.IsFileOpen())
   {
      packet.Clear();
      std::string_view file_name(packet.GetFileName());
      std::string_view content_type;
      if(file_name.find(".html") != std::string::npos)
      {
	 content_type = "Content-Type: text/html\r\n";
      }
      else if(file_name.find(".js") != std::string::npos)
      {
	 content_type = "Content-Type: text/javascript\r\n";
      }
      else if (file_name.find(".css") != std::string::npos)
      {
	 content_type = "Content-Type: text/css;\r\n";
      }
      else if (file_name.find(".mp4") != std::string::npos)
      {
	 content_type = "Content-Type: video/mp4\r\n";
      }
      
      std::string file_size = std::to_string(packet.GetFileSize());
      char range_str[] = { "Range: bytes=" };
      auto partial = request.find(range_str);
      if(partial != std::string::npos)
      {
	 auto end_offset = request.find("\r\n", partial)-(partial+sizeof(range_str)-1);

	 std::string partial_range(request.substr(partial+sizeof(range_str)-1, end_offset));
	 int num_left = 0;
	 int num_right = 0;

	 auto number_split = partial_range.find("-");
	 num_left = std::from_string<int>(partial_range.substr(0, number_split));
	 auto num_right_str = partial_range.substr(number_split+1, partial_range.npos);
	 if (!num_right_str.empty())
	 {
	    num_right = std::from_string<int>(num_right_str);	       
	 }
	 else
	 {
	    num_right = std::from_string<int>(file_size)-1;
	    partial_range.append(std::to_string(num_right));
	 }

	 int partial_bytes = (num_right - num_left)+1;
	 std::string content_length = std::to_string(partial_bytes);
	 if(partial_bytes > 0)
	 {
	    packet.IterWrite("HTTP/1.1 206 Partial Content\r\n");
	    packet.IterWrite(content_type.data(), content_type.length());
	    packet.IterWrite("Content-Length: ");
	    packet.IterWrite(content_length.data(), content_length.length());
	    packet.IterWrite("\r\n");
	    packet.IterWrite("Connection: Keep-Alive\r\n");
	    packet.IterWrite("Accept-Ranges: bytes\r\n");
	    packet.IterWrite("Content-Range: bytes ");
	    packet.IterWrite(partial_range.c_str(), partial_range.length());
	    packet.IterWrite(" / ");
	    packet.IterWrite(file_size.c_str(), file_size.length());
	    packet.IterWrite("\r\n\r\n");
	    start = num_left;
	    //byte ranges are inclusive! 0-1 = 2bytes
	    end = num_right+1;
	    packet.PrintDetails();
	    return 206;
	 }
	 else
	 {
	    packet.IterWrite("HTTP/1.1 416 Range Not Satisfiable\r\n");
	    packet.IterWrite(content_type.data(), content_type.length());
	    packet.IterWrite("Connection: Keep-Alive\r\n");
	    packet.IterWrite("Accept-Ranges: bytes\r\n");
	    packet.IterWrite("Content-Length: ");
	    packet.IterWrite(file_size.data(), file_size.length());
	    packet.IterWrite("\r\n\r\n");
	    packet.PrintDetails();
	    return 416;
	 }
      }
      else
      {
	 packet.IterWrite("HTTP/1.1 200 OK\r\n");
	 packet.IterWrite(content_type.data(), content_type.length());
	 packet.IterWrite("Connection: Keep-Alive\r\n");
	 packet.IterWrite("Accept-Ranges: bytes\r\n");
	 packet.IterWrite("Content-Length: ");
	 packet.IterWrite(file_size.data(), file_size.length());
	 packet.IterWrite("\r\n\r\n");
	 packet.PrintDetails();
	 return 200;
      }
   }
   return false;
}




