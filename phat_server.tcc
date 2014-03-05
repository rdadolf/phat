#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"

int chubby_port = 15810;
int puppet_port = 15808;

static Clp_Option options[] = {
  { "chubby-port", 'p', 0, Clp_ValInt, 0 },
  { "puppet-port", 'P', 0, Clp_ValInt, 0 },
};

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

  puppet::Puppet_Server puppet_server(puppet_port);
  //replica_server();
  //master_server(); // sleeping until consensus says I'm master

  tamer::loop();
  tamer::cleanup();
  return 0;
}
