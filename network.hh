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
bool get_ip_address( const char *hostname, struct in_addr &host );
bool get_ip_address( uint32_t ip_addr, struct in_addr &host );

#endif
