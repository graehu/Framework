#include "socket.h"
#include "../../utils/log/log.h"
#include <cerrno>
#include <cstdio>
#include <memory>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/signal.h>
#include <unistd.h>

using namespace fw;
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
   }
   return 0;
}
void socket::handle_signal_action(int sig_number)
{
   if (sig_number == SIGINT)
   {
      log::debug("SIGINT was caught, killing program");
     // shutdown_properly(EXIT_SUCCESS);
     exit(0);
   }
   else 
   if (sig_number == SIGPIPE)
   {
      log::debug("SIGPIPE was caught!");
      // shutdown_properly(EXIT_SUCCESS);
   }
}
socket::socket(Types _type) :
   m_socket(0),
   m_port(0),
   m_timeout(30),
   m_type(_type),
   mv_keepalive(false)
{
   if(setup_signals() == -1)
   {
      log::debug("need to shutdown sockets, something bad happened");
   }
}
socket::~socket()
{
   if(!mv_keepalive)
   {
      sleep(3);
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
   m_port = port;
   // create socket
   switch(m_type)
   {
      case eGameSocket:
	 log::debug("opening game socket");
	 m_socket = ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	 break;
      case eHttpSocket:
	 log::debug("opening http socket");
	 m_socket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	 break;
      case eAcceptSocket:
	 break;
   }

   if (m_socket <= 0)
   {
#if PLATFORM == PLATFORM_WINDOWS
      int error = WSAGetLastError();
      log::debug( "failed to open socket, error(%i)", error );
#else
      log::debug( "failed to open socket");
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
	    log::debug("error: failed to set feature levels on socket [%d]", m_socket);
	 }
#else
	 if(setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)))
	 {
	    log::debug("error: failed to set feature levels on socket [%d]", m_socket);
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
      log::debug( "failed to bind socket [%d]", m_socket);
      closeSock();
      return false;
   }
   
   // if(!mf_set_nonblocking(true))
   // {
   //    log::debug( "failed to set non-blocking socket" );
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
	    log::debug("failed to listen on socket");
	    return false;
	 }
	 break;
      case eAcceptSocket:
	 break;
   }
   log::debug("opened socket [%d] on port [%d]", m_socket, port);
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
      log::debug("closing socket [%d] on port [%d]", m_socket, m_port);
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

int socket::send(const address & destination, const void * data, int size)
{
   assert(data);
   assert(size > 0);

   if (m_socket == 0)
      return false;

   assert(destination.getAddress() != 0);
   assert(destination.getPort() != 0);
   struct timeval tv;
   tv.tv_sec = 0;
   tv.tv_usec = m_timeout;
   fd_set write_fds;
   FD_ZERO(&write_fds);
   FD_SET(STDOUT_FILENO, &write_fds);
   FD_SET(m_socket, &write_fds);
   
   int activity = ::select(m_socket+1, nullptr, &write_fds, nullptr, &tv);
   
   if(activity != 0 && activity != -1)
   {
      if(FD_ISSET(m_socket, &write_fds))
      {
         sockaddr_in address;
	 address.sin_family = AF_INET;
	 address.sin_addr.s_addr = htonl(destination.getAddress());
	 address.sin_port = htons((unsigned short) destination.getPort());
	 int sent_bytes = sendto(m_socket, (const char*)data, size, MSG_NOSIGNAL, (sockaddr*)&address, sizeof(sockaddr_in));
	 return sent_bytes;
      }
   }
   else
   {
      log::debug("I should shut down the socket.");  
   }
   return 0;

}

bool socket::Accept(address & sender, socket& _accept_socket)
{
   if(m_type == eHttpSocket)
   {
      struct timeval tv;
      tv.tv_sec = 60;
      tv.tv_usec = 0;
      sockaddr_in from;
      socklen_t fromLength = sizeof(from);
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(STDIN_FILENO, &read_fds);
      FD_SET(m_socket, &read_fds);

      int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, &tv);

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
	    _accept_socket.m_port = m_port;
	    _accept_socket.m_type = eAcceptSocket;

	    log::debug("opened socket [%d] on port [%d]", read_socket, m_port);
	    return true;
	 }
      }
      else if(activity == -1)
      {
	 log::debug("socket failed select: %s", strerror(errno));
      }
      return false;
   }
   else
   {
      log::debug("tried to accept on non http socket.");
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
   tv.tv_sec = 0;
   tv.tv_usec = m_timeout;
   if(setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0)
   {
      log::debug("couldn't set recieve timeout...");
   }
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

   int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, &tv);

   if(activity != 0 && activity != -1)
   {
      if (FD_ISSET(m_socket, &read_fds))
      {
	 // pollfd poll_fd;
	 // poll_fd.fd = m_socket; // your socket handler 
	 // poll_fd.events = POLLIN;
	 // int ret = poll(&poll_fd, 1, 1000); // 1 second for timeout
	 // log::debug("try post poll");
	 // if (ret > 0)
	 // {
	    int received_bytes = recvfrom(m_socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength);
	    if (received_bytes < 0)
	    {
	       log::debug("socket failed recvfrom: %s", strerror(errno));
	       return 0;
	    }
	    unsigned int nAddress = ntohl(from.sin_addr.s_addr);
	    unsigned short nPort = ntohs(from.sin_port);
	    sender = address(nAddress, nPort);
   
	    return received_bytes;	    
	 // }
	 // else if(ret == 0)
	 // {
	 //    log::debug("timed out on recieve!");
	 // }
	 // else
	 // {
	 //    log::debug("an error occured!");
	 // }
      }
      else
      {
	 log::debug("no read");
      }
   }
   else if(activity == -1)
   {
      log::debug("socket failed select: %s", strerror(errno));
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
   tv.tv_sec = 0;
   tv.tv_usec = m_timeout;
   if(setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0)
   {
      log::debug("couldn't set recieve timeout...");
   }
   int activity = ::select(m_socket+1, &read_fds, nullptr, nullptr, &tv);
   if(activity != 0 && activity != -1)
   {
      if (FD_ISSET(m_socket, &read_fds))
      {
	 // pollfd poll_fd;
	 // poll_fd.fd = m_socket; // your socket handler 
	 // poll_fd.events = POLLIN;
	 // int ret = poll(&poll_fd, 1, 1000); // 1 second for timeout

	 // if(ret > 0)
	 // {
	    int received_bytes = recv(m_socket, (char*)data, size, 0);
		 
	    if (received_bytes < 0)
	    {
	       log::debug("socket failed recv: %s", strerror(errno));
	       return 0;    
	    }
	    return received_bytes;
	 // }
      }
      else
      {
	 log::debug("no read");
      }
   }
   else if(activity == -1)
   {
      log::debug("socket failed select: %s", strerror(errno));
   }
   return 0;
}

