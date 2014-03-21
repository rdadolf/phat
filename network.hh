#ifndef _network_hh_
#define _network_hh_

#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "log.hh"

typedef struct {
  struct in_addr ip;
  int port;
} Server_t;


// One overloaded interface for ip lookup.
// This way, we can take whatever is in Json and just use it.
bool get_ip_address( const char *hostname, struct in_addr &host )
{
  in_addr_t ip;
  struct hostent *name_result;

  // Check if host was specified as "www.xxx.yyy.zzz"
  ip = inet_addr(hostname);
  if( ip!=INADDR_NONE ) {
    host.s_addr = ip;
    return true;
  }

  // Check if host was specified as "host.domain"
  name_result = gethostbyname(hostname);
  if( name_result==NULL || name_result->h_length!=4 || name_result->h_addrtype!=AF_INET ) {
    ERROR() << "Couldn't find hostname '" << hostname << "': " << strerror(errno);
    return false;
  }
  host = *((struct in_addr *) name_result->h_addr);
  return true;
}

bool get_ip_address( uint32_t ip_addr, struct in_addr &host )
{
  host.s_addr = ip_addr;
  return true;
}

#endif
