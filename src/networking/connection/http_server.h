#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <functional>
#include <iosfwd>
#include <string_view>
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
	 // #todo: move to using string_view and string here.
	 typedef std::function<void(const char*, size_t)> send_callback;
	 // #todo: consider uint opcodes, bool = text atm.
	 typedef std::function<void(const char*, size_t, bool)> ws_send_callback;
	 virtual void get_response(std::string_view request, send_callback send_cb) { }
	 virtual void post_response(std::string_view request, std::string_view body) { }
	 virtual void ws_response(const char* data, ws_send_callback send_cb) { }
	 virtual void ws_send(ws_send_callback send_cb) { }
	 virtual bool is_ws_handler() { return false; }
#pragma clang diagnostic pop
      };
      http_server(unsigned int port);
      ~http_server();
      // #todo: no way to remove handlers, no lifetime management
      void add_handler(handler* handler) { m_handlers.push_back(handler); }
      void set_root_dir(const char* root_dir) { m_root_dir = root_dir;}
   protected:
      // member functions
      void server_thread(const socket& listen);
      void ws_thread(const socket& from, const address& to);
      // member variables
      const char* m_root_dir;
      std::thread* m_server_thread;
      std::vector<handler*> m_handlers;
      std::vector<std::thread*> m_ws_threads;
      bool m_running;
   };
}
namespace fw
{
   namespace commandline
   {
      class http_handler : public net::http_server::handler
      {
      public:
	 void post_response(std::string_view request, std::string_view body) override;
      };
   }
}

#endif//HTTPCONNECTION_H
