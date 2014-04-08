// -*- mode: c++ -*-
#include "mpfd.hh"
#include "rpc_msg.hh"
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "paxos.hh"
#include "phat_server.hh"
using namespace paxos;

tamed void Paxos_Proposer::proposer_init (tamer::event<> done) {
    tvars {
        struct in_addr hostip;
        std::vector<int>::size_type i;
    }
    assert(ports.size() == mpfd.size());
    for (i = 0;i < ports.size(); ++i)
        twait { client_init(hostname.c_str(),ports[i],cfd[i],mpfd[i],hostip,make_event()); }
    done();
}

tamed void Paxos_Proposer::client_init(const char* hostname, int port, tamer::fd& cfd, 
                        msgpack_fd& mpfd, struct in_addr& hostip,tamer::event<> done) {

    tvars {
        int s = 100;
    }
    // lookup hostname address
    {
        in_addr_t a = hostname ? inet_addr(hostname) : htonl(INADDR_LOOPBACK);
        if (a != INADDR_NONE)
            hostip.s_addr = a;
        else {
            struct hostent* hp = gethostbyname(hostname);
            if (hp == NULL || hp->h_length != 4 || hp->h_addrtype != AF_INET) {
                std::cout << "lookup " << hostname << ": " << hstrerror(h_errno) << std::endl;
                return;
            }
            hostip = *((struct in_addr*) hp->h_addr);
        }
    }

    twait { tamer::tcp_connect(hostip, port, make_event(cfd)); }
    while (!cfd) {
        INFO() << "delaying: " << s;
        twait { tamer::at_delay_msec(s,make_event()); }
        twait { tamer::tcp_connect(hostip, port, make_event(cfd)); }
        if (s <= 10000)
            s *= 2;
        /*INFO() << "connect " << (hostname ? hostname : "localhost")
                  << ":" << port << ": " << strerror(-cfd.error()) << std::endl;
        return;*/
    }
    mpfd.initialize(cfd);
    done();
}

tamed void Paxos_Proposer::send_to_all(Json& req,tamer::event<> done){
    tvars {
        std::vector<int>::size_type i;
        tamer::rendezvous<int> r;
        int ret;
    }
    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i]));

    for (i = 0; i < (unsigned)(f + 1); ++i)
        twait(r,ret);
    
    done();
}

tamed void Paxos_Proposer::run_instance(Json _v,tamer::event<Json> done) {
    tvars {
        int n;
        std::vector<int>::size_type i;
        Json v;
        Json req;
        tamer::rendezvous<bool> r1;
        tamer::rendezvous<bool> r2;
        bool to;
    }
    set_vc(_v);
    INFO() << "starting instance";;
start:
    
    propose(n,v,r1.make_event(false));
    tamer::at_delay_sec(4,r1.make_event(true));
    twait(r1,to);
    if (to) { // timeout happened
        INFO() << "restarting after propose";;
        goto start;
    }

    if (v_o.empty()) {
        v_o = v_c;
        assert(!v_c.empty());
    }

    accept(n,r2.make_event(false));
    tamer::at_delay_sec(4,r2.make_event(true));
    twait(r2,to);
    if (to) {
        INFO() << "restarting after accept";;
        goto start;
    }
    
    req = Json::array(1,Json::null,DECIDED,n_p,v_o);
    twait { send_to_all(req,make_event()); }
    INFO() << "decided";;

    *done.result_pointer() = v_o;
    done.unblock();
}


tamed void Paxos_Proposer::propose(int n, Json v, tamer::event<> done) {
    tvars { 
        Json req;
        std::vector<int>::size_type i;
        tamer::rendezvous<int> r;
        int ret;
    }
    n_p = n_p + 1 + uid_; // FIXME : need uniqueifier 
    persist();
    n_o = a = 0;
    req = Json::array(1,Json::null,PREPARE,n_p);
    INFO() << "propose: " << req;;

    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i]));

    for (i = 0; i < (unsigned)f + 1; ++i) {
        twait(r,ret);
        assert(res[ret][2].is_i());
        if (res[ret][2].as_i() != PREPARED) 
            --i;
        else {
            assert(res[ret][3].is_i());
            n = res[ret][3].as_i();
            v = res[ret][4];
            if (n > n_o) {
                n_o = n;
                v_o = v;
            }
        }
    }

    done();
}

tamed void Paxos_Proposer::accept(int n, tamer::event<> done) {
    tvars {
        tamer::rendezvous<int> r;
        int ret;
        Json req;
        std::vector<int>::size_type i;
    }
    INFO() << "accept";;
    n_p = std::max(n_o,n_p);
    persist();
    req = Json::array(1,Json::null,ACCEPT,n_p,v_o);

    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i]));

    for (i = 0; i < (unsigned)(f + 1); ++i) {
        twait(r,ret);
        assert(res[ret][2].is_i() && res[ret][3].is_i());
        n = res[ret][3].as_i();
        if (res[ret][2].as_i() != ACCEPTED || n != n_p) // should not count this one
            --i;        
    }

    done();
}

tamed void Paxos_Acceptor::acceptor_init(tamer::event<> done) {
    tvars {
        tamer::fd sfd;
        tamer::fd cfd;
    }
    sfd = tamer::tcp_listen(port);
    if (sfd)
        INFO() << "listening on port " << port;
    else
        ERROR() << "listen: " << strerror(-sfd.error());
    while (sfd) {
        twait { sfd.accept(make_event(cfd)); }
        handle_request(cfd);
    }
    done();
}

tamed void Paxos_Acceptor::handle_request(tamer::fd cfd) {
    tvars {
        msgpack_fd mpfd(cfd);
        Json res,req = Json::make_array();
        int n;
        Json v;
    }
    while (cfd) {
        twait { mpfd.read_request(make_event(req)); }
        if (!req || !req.is_a() || req.size() < 4 || !req[0].is_i()
            || !req[1].is_i() || !req[2].is_i()) {

            INFO() << "bad RPC: " << req;;
            break;
        }
        switch(req[2].as_i()) {
            case PREPARE:
                INFO() << "prepare";;
                assert(req.size() == 4);
                n = req[3].as_i();
                prepare(mpfd,req,n);
                break;
            case ACCEPT:
                INFO() << "accept: " << req;;
                assert(req.size() == 5 && req[4].is_a());
                n = req[3].as_i();
                v = req[4];
                accept(mpfd,req,n,v);
                break;
            case DECIDED:
                INFO() << "decided";;
                assert(req.size() == 5 && req[4].is_a());
                v = req[4];
                decided(mpfd,req,v);
                break;
            default:
                INFO() << "bad Paxos request: " << req;;
                break;
        }
    }

    cfd.close();
}

tamed void Paxos_Acceptor::prepare(msgpack_fd& mpfd, Json& req,int n) {
    tvars {
        Json res = Json::make_array();
    }
    n_l = std::max(n_l,n);
    prepare_message(req,res,PREPARED);
    res[3] = n_a;
    res[4] = v_a;
    persist();
    mpfd.write(res);
}

tamed void Paxos_Acceptor::accept(msgpack_fd& mpfd, Json& req, int n, Json v) {
    tvars {
        Json res = Json::make_array();
    }
    if (n >= n_l) {
        n_l = n_a = n;
        v_a = v;
    }
    
    prepare_message(req,res,ACCEPTED);
    res[3] = n_a;
    persist();
    mpfd.write(res);
}

tamed void Paxos_Acceptor::decided(msgpack_fd& mpfd, Json& req,Json v) {
    tvars { Json res = Json::make_array(); }
    v_a = Json::make_array();
    if (v[0].is_s()) {
    if (v[0].as_s() == "master") {
        assert(v[1].is_i());
        me_->master_ = v[1].as_i();
    // } else (v[0].as_s() == "file") {
    }}
    persist();
    mpfd.write(res);
}

tamed void Paxos_Master::listen(tamer::event<> ev) {
    tvars {
        tamer::fd sfd;
        tamer::fd cfd;
    }
    sfd = tamer::tcp_listen(_port);
    if (sfd)
        INFO() << "listening on port " << _port;
    else
        ERROR() << "listen: " << strerror(-sfd.error());
    while (sfd) {
        twait { sfd.accept(make_event(cfd)); }
        handle_request(cfd);
    }
    ev();
}

tamed void Paxos_Master::handle_request(tamer::fd cfd) {
    tvars {
        msgpack_fd mpfd(cfd);
        RPC_Msg req,res;
    }
    while (cfd) {
        twait { mpfd.read_request(make_event(req.json())); }
        if (!req.content()[0].is_s() || req.content()[0].as_s() != "ports")
            res = RPC_Msg(Json::array("NACK"),req);
        else
            res = RPC_Msg(Json::array("ACK",_ports),req);
        INFO() << "handle paxos group request: " << res.content();
        mpfd.write(res);
    }
}
