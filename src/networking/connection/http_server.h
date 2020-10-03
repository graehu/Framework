#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <functional>
#include <vector>
namespace std
{
   class thread;
}
namespace net
{
   class socket;
   class address;
   class http_server
   {
   public:
      class handler
      {
      public:
	 // typedef void (*send_callback)(const char* message, size_t size);
	 typedef std::function<void(const char*, size_t)> send_callback;
	 virtual void mf_get_response(const char* request, send_callback send_cb) = 0;
	 virtual void mf_ws_response(const char* data, send_callback send_cb) = 0;
	 virtual void mf_ws_send(send_callback send_cb) = 0;
	 virtual const char* mf_get_response_root_dir() const = 0;
      };
      http_server(unsigned int port);
      ~http_server();
      // #todo: accept multiple handlers
      void mf_set_handler(handler* handler) { mv_handler = handler; }
      
   protected:
      // member functions
      void mf_server_thread(const socket& listen);
      void mf_ws_thread(const socket& from, const address& to);
      // member variables
      handler* mv_handler;
      std::thread* mv_server_thread;
      std::vector<std::thread*> mv_ws_threads;
      bool mv_running;
   };
}

#endif//HTTPCONNECTION_H
