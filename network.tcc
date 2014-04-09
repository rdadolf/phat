#include "network.hh"

const String& server_string(const Server_t& server)
{
  static String retval;
  retval  = String(inet_ntoa(server.ip));
  retval += String(":");
  retval += String(server.port);
  return retval;
}

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


////////////////////////////////////////////////////////////////////////////////
// modcomm functions

unsigned int modcomm_fd::delay_ = 0;

void modcomm_fd::initialize( tamer::fd cfd )
{
  mpfd_.initialize(cfd);
}

tamed void modcomm_fd::call( Json msg, tamer::event<Json> ev )
{
  tvars {
    Json reply;
  }

  INFO() << "(CALL1): " << msg;
  twait { tamer::at_delay_msec(modcomm_fd::delay_, make_event()); }
  twait { mpfd_.call( msg, make_event(reply) );  }
  // This second delay is the read delay on the reply
  // An RTT should be 2*delay_.
  twait { tamer::at_delay_msec(modcomm_fd::delay_, make_event()); }
  ev(reply);
}

tamed void modcomm_fd::write( Json msg, tamer::event<> ev )
{
  tvars {
    int x;
  }

  INFO() << "(WRITE): " << msg;
  twait { tamer::at_delay_msec(modcomm_fd::delay_, make_event()); }
  mpfd_.write( msg );

  ev();
}

tamed void modcomm_fd::read_request( tamer::event<Json> ev )
{
  tvars {
    Json response;
  }

  twait { mpfd_.read_request(make_event(response)); }
  twait { tamer::at_delay_msec(modcomm_fd::delay_, make_event()); }
  INFO() << "(READ): " << response;

  ev(response);
}
