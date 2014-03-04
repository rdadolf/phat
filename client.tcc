#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"
#include "api.hh"

int chubby_port = 15808;

static Clp_Option options[] = {
  { "chubby-port", 'p', 0, Clp_ValInt, 0 },
};

int main(int argc, char **argv)
{
  tamer::initialize();

  Clp_Parser *clp = Clp_NewParser(argc,argv,sizeof(options)/sizeof(options[0]),options);
  while(Clp_Next(clp)!=Clp_Done) {
    if(Clp_IsLong(clp, "chubby-port")) {
      chubby_port = clp->val.i;
    }
  }

  puppet_server();

  tamer::loop();
  tamer::cleanup();
  return 0;
}
