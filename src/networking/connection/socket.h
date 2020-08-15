#ifndef SOCKET_H
#define SOCKET_H

#include <memory>
#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
	#define PLATFORM PLATFORM_WINDOWS

#elif defined(__APPLE__)
	#define PLATFORM PLATFORM_MAC

#else
	#define PLATFORM PLATFORM_UNIX

#endif

#if PLATFORM == PLATFORM_WINDOWS

	#include <winsock2.h>
	#pragma comment( lib, "wsock32.lib" )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>

#else

	#error unknown platform!

#endif

#include <string.h>
#include <assert.h>
#include <stdio.h>

//this is here simply for the unistd include.

inline bool InitializeSockets()
{
	#if PLATFORM == PLATFORM_WINDOWS
   	WSADATA WsaData;
	int error = WSAStartup(MAKEWORD(2,2), &WsaData);
	return (error != 0);
	#else
	return false;
	#endif
}

inline void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}

#include "address.h"

namespace net
{

  class socket
  {
  public:
    enum Types
      {
       //m_socket = ::socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
       //eRawSocket, //Consider for a sniffer socket. 
       eGameSocket, //fast packets over udp
       eHttpSocket, //listen socket, for accept
       eAcceptSocket //temp socket returned from accept
      };

    socket(Types _types = eGameSocket);
    ~socket();
    bool Accept(address & _sender, socket & _accept_socket);
    bool mf_set_nonblocking(bool blocking);
      
    bool openSock(unsigned short port);
    void closeSock();
    bool IsOpen() const;
    bool send(const address & destination, const void * data, int size);
    int receive(void * data, int size);
    int receive(address & sender, void * data, int size);
    void mf_set_keepalive(bool keep_alive);
     bool mf_get_keepalive() { return mv_keepalive; }
     

  private:
    static int setup_signals();
    static void handle_signal_action(int signal_number);
    int m_socket;
     int m_port;
    Types m_type;
    bool mv_keepalive;
  };

}

#endif//SOCKET_H
