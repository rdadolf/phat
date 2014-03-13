#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"
#include "phat_server.hh"

using namespace phat;

int chubby_port = 15810;
int puppet_port = 15808;

static Clp_Option options[] = {
  { "chubby-port", 'p', 0, Clp_ValInt, 0 },
  { "puppet-port", 'P', 0, Clp_ValInt, 0 },
};

class Server_Puppet : public puppet::Puppet_Server
{
private:
  // FIXME: add lhs state for puppet scripts
public:
  Server_Puppet(int port) : Puppet_Server(port) {}

  void dispatch(String tag, Json args);

  tamed void service_electme(Json args);
};


void Server_Puppet::dispatch(String tag, Json args)
{
  puppet::Puppet_Server::dispatch(tag, args);

  if(tag=="electme")
    service_electme(args);
  // No warning about unknown messages, to allow extensibility via inheritance.
}

tamed void Server_Puppet::service_electme(Json args)
{

}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  tamer::initialize();

  Clp_Parser *clp = Clp_NewParser(argc,argv,sizeof(options)/sizeof(options[0]),options);
  while(Clp_Next(clp)!=Clp_Done) {
    if(Clp_IsLong(clp, "chubby-port")) {
      chubby_port = clp->val.i;
    }
    else if(Clp_IsLong(clp, "puppet-port")) {
      puppet_port = clp->val.i;
    }
  }

  Server_Puppet puppet_server(puppet_port);
  Phat_Server phat(chubby_port);

  tamer::loop();
  tamer::cleanup();
  return 0;
}


