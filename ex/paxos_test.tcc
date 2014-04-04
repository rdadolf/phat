// -*- mode: c++ -*-
#include "clp.h"
#include "mpfd.hh"
#include "paxos.hh"
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

tamed void run_acceptor_test(int port) {
    tvars {
        Paxos_Acceptor::paccept_type* pa;
    }
    pa = new Paxos_Acceptor(port);
    twait { pa->acceptor_init(make_event()); }
    delete pa;
}

tamed void run_proposer_test(int port, int f) {
    tvars {
        Paxos_Proposer::ppropose_type* pp;
        std::vector<int> ports;
        std::vector<int>::size_type i;
        std::vector<int>::size_type n;
        Json req,ret;
    }
    n = 2 * f + 1;
    for (i = 0 ; i < n ; ++i)
        ports.push_back(port + i);

    pp = new Paxos_Proposer("localhost",ports,f);
    req = Json::array("abs");
    twait { pp->proposer_init(make_event()); }
    twait { pp->run_instance(req,make_event(ret)); }
    std::cout << "Chose: " << ret << std::endl;
    delete pp;
}

tamed void run_both_test(int port, int f) {
    tvars {
        Paxos_Acceptor::paccept_type* pa;
        Paxos_Proposer::ppropose_type* pp;
        std::vector<int> ports;
        std::vector<int>::size_type i;
        std::vector<int>::size_type n;
        std::vector<int> children;
        int pid;
        Json ret,req;
    }
    n = 2 * f + 1;
    for (i = 0; i < n; ++i) {
        ports.push_back(port + i);
        if ((pid = fork()) < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            pa = new Paxos_Acceptor(port + i);
            twait { pa->acceptor_init(make_event()); }
            delete pa;
            exit(0);
        }
        children.push_back(pid);
    }

    usleep(1000000); // introduce delay to make sure we have the acceptors and running
    pp = new Paxos_Proposer("localhost",ports,f);
    req = Json::array("abs");
    twait { pp->proposer_init(make_event()); }
    twait { pp->run_instance(req,make_event(ret)); }

    std::cout << "Chose " << ret << std::endl;

    for (i = 0; i < children.size(); ++i)
        kill(children[i],SIGTERM);
    children.clear();

    delete pp;
}

static Clp_Option options[] = {
    { "acceptor", 'a', 0, Clp_ValInt, 0 },
    { "proposer", 'p', 0, 0, 0 },
    { "f",'f',0,Clp_ValInt,0},
    { "both" ,'b',0,Clp_ValInt,0}
};

int main(int argc, char** argv) {
    tamer::initialize();

    bool is_proposer = false;
    bool is_both = false;
    int port = 18029;
    int f = 0;
    Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

    while (Clp_Next(clp) != Clp_Done) {
        if (Clp_IsLong(clp, "proposer"))
            is_proposer = true;
        else if (Clp_IsLong(clp, "acceptor")) {
            if(clp->have_val)
                port = clp->val.i;
        }
        else if (Clp_IsLong(clp,"both")) {
            if(clp->have_val)
                f = clp->val.i;
            is_both = true;
        }
        else if (Clp_IsLong(clp,"f"))
            f = clp->val.i;
    }
    Clp_DeleteParser(clp);

    if (is_both)
        run_both_test(port,f);
    else if(is_proposer)
        run_proposer_test(port, f);
    else 
        run_acceptor_test(port);

    tamer::loop();
    tamer::cleanup();
}
