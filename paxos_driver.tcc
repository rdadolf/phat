#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "paxos.hh"

using namespace paxos;

int port = 15800;
int port_start = 15900;
int n = 3;

static Clp_Option options[] = {
  { "port", 'p', 0, Clp_ValInt, 0 },
  { "port_start", 'P', 0, Clp_ValInt, 0 },
  { "number", 'n', 0, Clp_ValInt, 0 },
};

tamed void run(Paxos_Master pm) {
  twait { pm.listen(make_event()); }
}

int main(int argc, char const *argv[]) {
    tamer::initialize();
    Clp_Parser *clp = Clp_NewParser(argc,argv,sizeof(options)/sizeof(options[0]),options);
    while(Clp_Next(clp)!=Clp_Done) {
      if(Clp_IsLong(clp, "port")) {
        port = clp->val.i;
      } else if(Clp_IsLong(clp, "number")) {
        n = clp->val.i;
      }
    }

    INFO() << "Paxos Master Driver is set up at PID " << getpid();

    std::vector<int> ports;
    for (int i = 0; i < n; i++)
        ports.push_back(port_start + i);

    Paxos_Master pm(port,ports);
    run(pm);

    tamer::loop();
    tamer::cleanup();
    return 0;
}
