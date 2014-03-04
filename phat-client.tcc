#include "phat.hh"
#include <tamer/fd.hh>
#include <netdb.h>

using tamer::event;
using std::vector;

namespace phat {

Phat::Phat() : mpfd_(nullptr) {
}

tamed void Phat::connect(String host, uint32_t port, event<> done) {
    tvars {
        tamer::fd fd;
        struct in_addr hostip;
    }

    assert(!mpfd_);
    assert(host);

    // lookup hostname address
    {
        in_addr_t a = inet_addr(host.c_str());
        if (a != INADDR_NONE)
            hostip.s_addr = a;
        else {
            struct hostent* hp = gethostbyname(host.c_str());
            if (hp == NULL || hp->h_length != 4 || hp->h_addrtype != AF_INET) {
                std::cerr << "lookup " << host << ": " << hstrerror(h_errno) << std::endl;
                return;
            }
            hostip = *((struct in_addr*) hp->h_addr);
        }
    }

    // connect
    twait { tamer::tcp_connect(hostip, port, make_event(fd)); }
    if (!fd) {
        std::cerr << "connect " << host << ":" << port << ": " 
                  << strerror(-fd.error()) << std::endl;
        return;
    }

    mpfd_ = new msgpack_fd(fd);
    done();
}

tamed void Phat::disconnect(event<> done) {
    done();
}

tamed void Phat::getRoot(event<Handle*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::open(Handle* handle, String subpath, 
                      event<Handle*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::mkfile(Handle* handle, String subpath, String initialdata, 
                        event<Handle*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::mkdir(Handle* handle, String subpath, 
                       event<Handle*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::getcontents(Handle* handle, event<String, int32_t> done) {
    done("", -1);
}

tamed void Phat::putcontents(Handle* handle, String data, event<int32_t> done) {
    done(-1);
}

tamed void Phat::readdir(Handle* handle, event<std::vector<String>, int32_t> done) {
    done(vector<String>(), -1);
}

tamed void Phat::stat(Handle* handle, event<Metadata*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::flock(Handle* handle, const LockType& type, 
                       event<Sequencer*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::funlock(Handle* handle, event<Sequencer*, int32_t> done) {
    done(nullptr, -1);
}

tamed void Phat::remove(Handle* handle, event<int32_t> done) {
    done(-1);
}

}
