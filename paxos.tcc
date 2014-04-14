// -*- mode: c++ -*-
#include "mpfd.hh"
#include "rpc_msg.hh"
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "paxos.hh"
#include "network.hh"
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
                        modcomm_fd& mpfd, struct in_addr& hostip,tamer::event<> done) {

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
        INFO() << "delaying to connect to " << port << ": " << s;
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

tamed void Paxos_Proposer::send_to_all(RPC_Msg& req){
    tvars {
        std::vector<int>::size_type i;
        tamer::rendezvous<int> r;
        int ret;
    }
    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i].json()));
/*
    for (i = 0; i < (unsigned)(f + 1); ++i)
        twait(r,ret);
    
    done();*/
}

tamed void Paxos_Proposer::run_instance(Json _v,tamer::event<Json> done) {
    tvars {
        int n;
        std::vector<int>::size_type i;
        Json v;
        RPC_Msg req;
        tamer::rendezvous<bool> r;
        bool to;
    }
    set_vc(_v);
    INFO() << "starting instance";;
start:
    if(v_c[1].as_i() != me_->epoch_) { // replica is no longer master: shouldn't be sending 
        assert(v_c[1].as_i() < me_->epoch_);
        INFO() << "proposer's epoch number is behind in run_instance";
        v_o = Json::array("NACK");
        goto done;
    }
    propose(n,v,r.make_event(false));
    tamer::at_delay_sec(4,r.make_event(true));
    twait(r,to);
    if (to) { // timeout happened
        INFO() << "restarting after propose";;
        goto start;
    }
    r.clear();
    if (v_o.empty()) {
        v_o = v_c;
        assert(!v_c.empty());
    }

    accept(n,r.make_event(false));
    tamer::at_delay_sec(4,r.make_event(true));
    twait(r,to);
    if (to) {
        INFO() << "restarting after accept";;
        goto start;
    }
    
    req = RPC_Msg(Json::array(DECIDED,v_c[1].as_i(),n_p,v_o));
    send_to_all(req);
    INFO() << "decided" << req.content();;
done:
    *done.result_pointer() = v_o;
    done.unblock();
}


tamed void Paxos_Proposer::propose(int n, Json v, tamer::event<> done) {
    tvars { 
        RPC_Msg req;
        std::vector<int>::size_type i;
        tamer::rendezvous<int> r;
        int ret;
    }
    n_p = n_p + 1 + uid_; // FIXME : need uniqueifier 
    persist();
    n_o = a = 0;
    req = RPC_Msg(Json::array(PREPARE,v_c[1].as_i(),n_p));
    INFO() << "propose: " << req.content();;

    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i].json()));

    for (i = 0; i < (unsigned)f + 1; ++i) {
        twait(r,ret);
        assert(res[ret].content()[0].is_i());
        if (res[ret].content()[0].as_i() != PREPARED) 
            --i;
        else {
            assert(res[ret].content()[1].is_i());
            n = res[ret].content()[1].as_i();
            v = res[ret].content()[2];
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
        RPC_Msg req;
        std::vector<int>::size_type i;
    }
    INFO() << "accept";;
    n_p = std::max(n_o,n_p);
    persist();
    req = RPC_Msg(Json::array(ACCEPT,v_c[1].as_i(),n_p,v_o));

    for (i = 0; i < ports.size(); ++i)
        mpfd[i].call(req,r.make_event(i,res[i].json()));

    for (i = 0; i < (unsigned)(f + 1); ++i) {
        twait(r,ret);
        assert(res[ret].content()[0].is_i() && res[ret].content()[1].is_i());
        n = res[ret].content()[1].as_i();
        if (res[ret].content()[0].as_i() != ACCEPTED || n != n_p) // should not count this one
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
        modcomm_fd mpfd(cfd);
        RPC_Msg res,req;
        int n;
        Json v;
    }
    while (cfd) {
        twait { mpfd.read_request(make_event(req.json())); }
        if (!req.content().is_a() || req.content().size() <= 2
            || !req.content()[0].is_i() || !req.content()[1].is_i()) {
            INFO() << "bad RPC: " << req.content();;
            break;
        }
        if (me_->epoch_ != req.content()[1].as_i()) {// ignore request; proposer should time out and realize it's behind
            INFO() << "proposer's epoch number is behind in acceptor";
            continue;
        }
        // heartbeat
        me_->elect_me_.make_event(false).trigger();
        switch(req.content()[0].as_i()) {
            case PREPARE:
                INFO() << "prepare: " << req.content();;
                assert(req.content().size() == 3);
                n = req.content()[2].as_i();
                prepare(mpfd,req,n);
                break;
            case ACCEPT:
                INFO() << "accept: " << req.content();;
                assert(req.content().size() == 4 && req.content()[3].is_a());
                n = req.content()[2].as_i();
                v = req.content()[3];
                accept(mpfd,req,n,v);
                break;
            case DECIDED:
                INFO() << "decided: " << req.content();;
                assert(req.content().size() == 4 && req.content()[3].is_a());
                v = req.content()[3];
                decided(mpfd,req,v);
                break;
            default:
                INFO() << "bad Paxos request: " << req.content();;
                break;
        }
    }

    cfd.close();
}

tamed void Paxos_Acceptor::prepare(modcomm_fd& mpfd, RPC_Msg& req,int n) {
    tvars {
        RPC_Msg res;
    }
    n_l = std::max(n_l,n);
    res = RPC_Msg(Json::array(PREPARED,n_a,v_a),req);
    persist();
    mpfd.write(res);
}

tamed void Paxos_Acceptor::accept(modcomm_fd& mpfd, RPC_Msg& req, int n, Json v) {
    tvars {
        RPC_Msg res;
    }
    if (n >= n_l) {
        n_l = n_a = n;
        v_a = v;
    }
    
    res = RPC_Msg(Json::array(ACCEPTED,n_a),req);
    persist();
    mpfd.write(res);
}

tamed void Paxos_Acceptor::decided(modcomm_fd& mpfd, RPC_Msg& req,Json v) {
    tvars { 
        RPC_Msg res; 
    }
    v_a = Json::make_array();
    if (v[0].is_s()) {
    if (v[0].as_s() == "master") {
        assert(v[1].is_i() && v[2].is_i());
        me_->master_ = v[2].as_i();
        me_->epoch_ = v[1].as_i() + 1;
    // } else (v[0].as_s() == "file") {
    }}
    INFO() << "I, "<< me_->paxos_port_ << ", think I am master: " << me_->i_am_master();
    INFO () << "master is: " << me_->master_;
    res = RPC_Msg(Json::array(DECIDED,"ACK"),req);
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
