#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"
#include "phat_server.hh"

using namespace phat;

int phat_port = 15810;
int puppet_port = 15808;

static Clp_Option options[] = {
  { "phat-port", 'p', 0, Clp_ValInt, 0 },
  { "puppet-port", 'P', 0, Clp_ValInt, 0 },
};

class Server_Puppet : public puppet::Puppet_Server
{
private:
  // FIXME: add lhs state for puppet scripts
  Phat_Server phat_;
public:
  Server_Puppet(int puppet_port, int phat_port) : Puppet_Server(puppet_port), phat_(phat_port) {}

  virtual void dispatch(String tag, Json args, tamer::event<> ev);

  tamed void service_electme(Json args, tamer::event<> ev);
};


void Server_Puppet::dispatch(String tag, Json args, tamer::event<> ev)
{
  puppet::Puppet_Server::dispatch(tag, args, ev);

  if(tag=="electme")
    service_electme(args, ev);
  // No warning about unknown messages, to allow extensibility via inheritance.
}

tamed void Server_Puppet::service_electme(Json args, tamer::event<> ev)
{
  // FIXME: NYI

  ev();
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  tamer::initialize();

  Clp_Parser *clp = Clp_NewParser(argc,argv,sizeof(options)/sizeof(options[0]),options);
  while(Clp_Next(clp)!=Clp_Done) {
    if(Clp_IsLong(clp, "phat-port")) {
      phat_port = clp->val.i;
    }
    else if(Clp_IsLong(clp, "puppet-port")) {
      puppet_port = clp->val.i;
    }
  }

  INFO() << "Server Driver up at PID " << getpid() << std::endl;
  Server_Puppet puppet_server(puppet_port, phat_port);

  tamer::loop();
  tamer::cleanup();
  return 0;
}


