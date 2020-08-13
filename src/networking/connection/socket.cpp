#include "socket.h"
#include <cerrno>
#include <cstdio>
#include <memory>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/signal.h>
#include <unistd.h>
#define show_val(variable) printf(#variable": %d\n", variable);
#define show_str(variable) printf(#variable": %s\n", variable);
using namespace net;

int socket::setup_signals()
{
   static bool do_once = true;
   if(do_once)
   {
      struct sigaction sa;
      sa.sa_handler = socket::handle_signal_action;
      if (sigaction(SIGINT, &sa, 0) != 0) {
	 perror("sigaction()");
	 return -1;
      }
      if (sigaction(SIGPIPE, &sa, 0) != 0) {
	 perror("sigaction()");
	 return -1;
      }
      do_once = false;
      printf("set up signals handler\n");
   }
  return 0;
}
void socket::handle_signal_action(int sig_number)
{
   // if (sig_number == SIGINT)
   // {
   //   printf("SIGINT was caught!\n");
   //   // shutdown_properly(EXIT_SUCCESS);
   // }
   // else 
   if (sig_number == SIGPIPE)
   {
      printf("SIGPIPE was caught!\n");
      // shutdown_properly(EXIT_SUCCESS);
   }
}
socket::socket(Types _type) :
   m_socket(0),
   m_type(_type),
   mv_keepalive(false)
{
   if(setup_signals() == -1)
   {
      printf("need to shutdown sockets, something bad happened");
   }
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
      case eAcceptSocket:
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
      case eAcceptSocket:
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
   
   // if(!mf_set_nonblocking(true))
   // {
   //    printf( "failed to set non-blocking socket\n" );
   //    closeSock();
   //    return false;
   // }

   
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
      case eAcceptSocket:
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
      printf("closing socket...\n");
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
   fd_set write_fds;
   FD_ZERO(&write_fds);
   FD_SET(STDOUT_FILENO, &write_fds);
   FD_SET(m_socket, &write_fds);
   int activity = ::select(m_socket+1, nullptr, &write_fds, nullptr, nullptr);
   
   if(activity != 0 && activity != -1)
   {
      if(FD_ISSET(m_socket, &write_fds))
      {
         sockaddr_in address;
	 address.sin_family = AF_INET;
	 address.sin_addr.s_addr = htonl(destination.getAddress());
	 address.sin_port = htons((unsigned short) destination.getPort());
	 int sent_bytes = sendto(m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in));
	 return sent_bytes == size;
      }
      else
      {
	 //socket in use, that's probably fine
	 // printf("hmm lets hope this doesn't happen too much..\n");
      }
   }
   else
   {
      printf("I should shut down the socket.\n");  
   }
   return false;

}

bool socket::Accept(address & sender, socket& _accept_socket)
{
   if(m_type == eHttpSocket)
   {
      sockaddr_in from;
      socklen_t fromLength = sizeof(from);
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(STDIN_FILENO, &read_fds);
      FD_SET(m_socket, &read_fds);

      int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, nullptr);

      if(activity != 0 && activity != -1)
      {
	 if (FD_ISSET(m_socket, &read_fds))
	 {
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
      }
      else
      {
	 printf("I should shut down the socket.\n");  
      }
      return false;
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
   //todo put this into a function.
   struct timeval tv;
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
//todo add these to the function
// // WINDOWS
// DWORD timeout = timeout_in_seconds * 1000;
// setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

// // MAC OS X (identical to Linux)
// struct timeval tv;
// tv.tv_sec = timeout_in_seconds;
// tv.tv_usec = 0;
// setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

   sockaddr_in from;
   socklen_t fromLength = sizeof(from);
   fd_set read_fds;
   FD_ZERO(&read_fds);
   FD_SET(STDIN_FILENO, &read_fds);
   FD_SET(m_socket, &read_fds);

   int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, nullptr);

   if(activity != 0 && activity != -1)
   {
      if (FD_ISSET(m_socket, &read_fds))
      {
	 printf("try recvfrom\n");
	 pollfd poll_fd;
	 poll_fd.fd = m_socket; // your socket handler 
	 poll_fd.events = POLLIN;
	 int ret = poll(&poll_fd, 1, 1000); // 1 second for timeout
	 if (ret > 0)
	 {
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
	 else if(ret == 0)
	 {
	    printf("timed out on recieve!\n");
	 }
	 else
	 {
	    printf("an error occured!\n");
	 }
      }
      else
      {
	 printf("no read\n");
      }
   }
   else
   {
      printf("I should shut down the socket.\n");  
   }
   return 0;
}
int socket::receive(void * data, int size)
{
   assert(data);
   assert(size > 0);
   assert(IsOpen());
   fd_set read_fds;
   FD_ZERO(&read_fds);
   FD_SET(STDIN_FILENO, &read_fds);
   FD_SET(m_socket, &read_fds);
   //todo put this into a function.
   struct timeval tv;
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
   

   int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, nullptr);

   if(activity != 0 && activity != -1)
   {
      if (FD_ISSET(m_socket, &read_fds))
      {
	 int received_bytes = recv(m_socket, (char*)data, size, 0);
	 if (received_bytes <= 0)
	 {
	    return 0;    
	 }
	 return received_bytes;
      }
      else
      {
	 printf("no read\n");	 
      }
   }
   return 0;
}

