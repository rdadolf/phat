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

  virtual void dispatch(String tag, Json args, tamer::event<> ev);

  tamed void service_getroot(Json args, tamer::event<> ev);
  tamed void service_open(Json args, tamer::event<> ev);
  tamed void service_mkfile(Json args, tamer::event<> ev);
  tamed void service_mkdir(Json args, tamer::event<> ev);
  tamed void service_getcontents(Json args, tamer::event<> ev);
  tamed void service_putcontents(Json args, tamer::event<> ev);
  tamed void service_readdir(Json args, tamer::event<> ev);
  tamed void service_stat(Json args, tamer::event<> ev);
  tamed void service_flock(Json args, tamer::event<> ev);
  tamed void service_unlock(Json args, tamer::event<> ev);
  tamed void service_remove(Json args, tamer::event<> ev);
};


// Called by the puppet server class's internals.
// Extensible dispatch.
void Client_Puppet::dispatch(String tag, Json args, tamer::event<> ev)
{
  puppet::Puppet_Server::dispatch(tag, args, ev);

  if(tag=="puppet_getroot")
    service_getroot(args, ev);
  else if(tag=="puppet_open")
    service_open(args, ev);
  else if(tag=="puppet_mkfile")
    service_mkfile(args, ev);
  else if(tag=="puppet_mkdir")
    service_mkdir(args, ev);
  else if(tag=="puppet_getcontents")
    service_getcontents(args, ev);
  else if(tag=="puppet_putcontents")
    service_putcontents(args, ev);
  else if(tag=="puppet_readdir")
    service_readdir(args, ev);
  else if(tag=="puppet_stat")
    service_stat(args, ev);
  else if(tag=="puppet_flock")
    service_flock(args, ev);
  else if(tag=="puppet_unlock")
    service_unlock(args, ev);
  else if(tag=="puppet_remove")
    service_remove(args, ev);

}

tamed void Client_Puppet::service_getroot(Json args, tamer::event<> ev )
{
  tvars {
    Handle h;
  }

  INFO() << "Client servicing getroot request";

  twait{ phat_.getroot(make_event(h)); }

  // FIXME: storage for h is specified by lhs in args, do something with it
  ev();
}

tamed void Client_Puppet::service_open(Json args, tamer::event<> ev )
{
  ev();
}

tamed void Client_Puppet::service_mkfile(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing mkfile request";

  twait{ phat_.mkfile(args,make_event(h)); }
  ev();
}

tamed void Client_Puppet::service_mkdir(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing mkdir request";

  twait{ phat_.mkdir(args,make_event(h)); }
  ev(); 
}

tamed void Client_Puppet::service_getcontents(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing getcontents request";

  twait{ phat_.getcontents(args,make_event(h)); }
  ev(); 
}

tamed void Client_Puppet::service_putcontents(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing putcontents request";

  twait{ phat_.putcontents(args,make_event(h)); }
  ev(); 
}

tamed void Client_Puppet::service_readdir(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing readdir request";

  twait{ phat_.readdir(args,make_event(h)); }
  ev(); 
}

tamed void Client_Puppet::service_stat(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing stat request";

  twait{ phat_.stat(args,make_event(h)); }
  ev(); 
}

tamed void Client_Puppet::service_flock(Json args, tamer::event<> ev)
{
  ev();
}

tamed void Client_Puppet::service_unlock(Json args, tamer::event<> ev)
{
  ev();
}

tamed void Client_Puppet::service_remove(Json args, tamer::event<> ev)
{
  tvars {
    Handle h;
  }
  INFO() << "Client servicing remove request";

  twait{ phat_.remove(args,make_event(h)); }
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

  INFO() << "Client Driver up at PID " << getpid();
  Client_Puppet puppet_server(puppet_port);

  tamer::loop();
  tamer::cleanup();
  return 0;
}
