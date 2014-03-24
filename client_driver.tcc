#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"
#include "phat_api.hh"

using namespace phat;

int phat_port = 15810;
int puppet_port = 15808;

static Clp_Option options[] = {
  { "phat-port", 'p', 0, Clp_ValInt, 0 },
  { "puppet-port", 'P', 0, Clp_ValInt, 0 },
};

class Client_Puppet : public puppet::Puppet_Server
{
private:
  // FIXME: add lhs state for puppet scripts
  Phat_Interface phat_;
public:
  Client_Puppet(int puppet_port) : Puppet_Server(puppet_port) {}

  void dispatch(String tag, Json args);

  tamed void service_getroot(Json args);
  tamed void service_open(Json args);
  tamed void service_mkfile(Json args);
  tamed void service_mkdir(Json args);
  tamed void service_getcontents(Json args);
  tamed void service_putcontents(Json args);
  tamed void service_readdir(Json args);
  tamed void service_stat(Json args);
  tamed void service_flock(Json args);
  tamed void service_unlock(Json args);
  tamed void service_remove(Json args);
};


// Called by the puppet server class's internals.
// Extensible dispatch.
void Client_Puppet::dispatch(String tag, Json args)
{
  puppet::Puppet_Server::dispatch(tag, args);

  if(tag=="getroot")
    service_getroot(args);
  else if(tag=="open")
    service_open(args);
  else if(tag=="mkfile")
    service_mkfile(args);
  else if(tag=="mkdir")
    service_mkdir(args);
  else if(tag=="getcontents")
    service_getcontents(args);
  else if(tag=="putcontents")
    service_putcontents(args);
  else if(tag=="readdir")
    service_readdir(args);
  else if(tag=="stat")
    service_stat(args);
  else if(tag=="flock")
    service_flock(args);
  else if(tag=="unlock")
    service_unlock(args);
  else if(tag=="remove")
    service_remove(args);
  // No warning about unknown messages, to allow extensibility via inheritance.
}

tamed void Client_Puppet::service_getroot(Json args)
{
  Handle h;
  h = phat_.getroot();
  // FIXME: storage for h is specified by lhs in args, do something with it
}

tamed void Client_Puppet::service_open(Json args)
{

}

tamed void Client_Puppet::service_mkfile(Json args)
{

}

tamed void Client_Puppet::service_mkdir(Json args)
{

}

tamed void Client_Puppet::service_getcontents(Json args)
{

}

tamed void Client_Puppet::service_putcontents(Json args)
{

}

tamed void Client_Puppet::service_readdir(Json args)
{

}

tamed void Client_Puppet::service_stat(Json args)
{

}

tamed void Client_Puppet::service_flock(Json args)
{

}

tamed void Client_Puppet::service_unlock(Json args)
{

}

tamed void Client_Puppet::service_remove(Json args)
{

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

  Client_Puppet puppet_server(puppet_port);

  tamer::loop();
  tamer::cleanup();
  return 0;
}
