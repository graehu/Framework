#include "socket.h"
#include <cstdio>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define show_val(variable) printf(#variable": %d\n", variable);
#define show_str(variable) printf(#variable": %s\n", variable);
using namespace net;

socket::socket(Types _type) :
   m_socket(0),
   m_type(_type),
   mv_keepalive(false)
{
}
socket::~socket()
{
   if(!mv_keepalive)
   {
      closeSock();      
   }
}
void socket::mf_set_keepalive(bool keep_alive)
{
  mv_keepalive = keep_alive;
}
bool socket::openSock(unsigned short port)
{
   assert(!IsOpen());

   // create socket
   switch(m_type)
   {
      case eGameSocket:
	 printf("making game socket\n");
	 m_socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	 break;
      case eHttpSocket:
	 printf("making web socket\n");
	 m_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	 break;
   }

   if (m_socket <= 0)
   {
#if PLATFORM == PLATFORM_WINDOWS
      int error = WSAGetLastError();
      printf( "failed to create socket, error(%i)\n", error );
#else
      printf( "failed to create socket\n");
#endif
      m_socket = 0;
      return false;
   }

   switch(m_type)
   {
      case eHttpSocket:
      {
	 int on = 1;
#if PLATFORM == PLATFORM_WINDOWS
	 if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)))
	 {
	    printf("error: failed to set feature levels\n");		    
	 }
#else
	 if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)))
	 {
	    printf("error: failed to set feature levels\n");
	 }
#endif
      }
      break;
      case eGameSocket:
	 break;
   } 

	
   // bind to port
   sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(port);

   if (bind(m_socket, (const sockaddr*) &address, sizeof(sockaddr_in)) != 0)
   {
      printf( "failed to bind socket\n" );
      closeSock();
      return false;
   }
   if(!mf_set_nonblocking(true))
   {
      printf( "failed to set non-blocking socket\n" );
      closeSock();
      return false;
   }

   switch(m_type)
   {
      case eGameSocket:
	 break;
      case eHttpSocket:
	 //TODO: do something about this random limit here.
	 if(listen(m_socket, 8) < 0)
	 {
	    printf("failed to listen on socket\n");
	    return false;
	 }
	 break;
   }
   printf("bound socket: %d\n", port);
   return true;
}
bool socket::mf_set_nonblocking(bool non_blocking)
{
     // set non-blocking io
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

   int nonBlocking = non_blocking ? 1 : 0;
   if ( fcntl(m_socket, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 )
   {
      return false;
   }

#elif PLATFORM == PLATFORM_WINDOWS

   DWORD nonBlocking = non_blocking ? 1 : 0;
   if ( ioctlsocket( m_socket, FIONBIO, &nonBlocking ) != 0 )
   {
      return false;
   }
#endif
   return true;
}

void socket::closeSock()
{
   if (m_socket != 0)
   {
     //printf("closing socket...\n");
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
      close(m_socket);
#elif PLATFORM == PLATFORM_WINDOWS
      closesocket(m_socket);
#endif
      m_socket = 0;
   }
}

bool socket::IsOpen() const
{
   return m_socket != 0 || m_type == eAcceptSocket;
}

bool socket::send(const address & destination, const void * data, int size)
{
   assert(data);
   assert(size > 0);

   if (m_socket == 0)
      return false;

   assert(destination.getAddress() != 0);
   assert(destination.getPort() != 0);

   sockaddr_in address;
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = htonl(destination.getAddress());
   address.sin_port = htons((unsigned short) destination.getPort());
   int sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));
   return sent_bytes == size;
}

bool socket::Accept(address & sender, socket& _accept_socket)
{
   if(m_type == eHttpSocket)
   {
      sockaddr_in from;
      socklen_t fromLength = sizeof(from);
      int read_socket = ::accept(m_socket, (sockaddr*)&from, &fromLength);
      if(read_socket <= 0)
      {
	 return false;
      }
      unsigned int nAddress = ntohl(from.sin_addr.s_addr);
      unsigned short nPort = ntohs(from.sin_port);
      sender = address(nAddress, nPort);
      _accept_socket.m_socket = read_socket;
      _accept_socket.m_type = eAcceptSocket;
      return true;
   }
   else
   {
      printf("tried to accept on non http socket.\n");
   }
   return false;
}


int socket::receive(address & sender, void * data, int size)
{
   assert(data);
   assert(size > 0);
   assert(IsOpen());

#if PLATFORM == PLATFORM_WINDOWS
   typedef int socklen_t;
#endif

   sockaddr_in from;
   socklen_t fromLength = sizeof(from);
   // show_val(data);
   // show_val(size);
   show_val(m_socket);
   int received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength);
   read(m_socket, (char*)data, size);
   if (received_bytes <= 0)
   {
      return 0;    
   }
   unsigned int nAddress = ntohl(from.sin_addr.s_addr);
   unsigned short nPort = ntohs(from.sin_port);
   sender = address(nAddress, nPort);
   
   return received_bytes;
}
int socket::receive(void * data, int size)
{
   assert(data);
   assert(size > 0);
   assert(IsOpen());
   
   int received_bytes = recv(m_socket, (char*)data, size, 0);
   if (received_bytes <= 0)
   {
      return 0;    
   }
   return received_bytes;
}

