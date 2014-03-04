#include "phat-rpc.hh"
#include "mpfd.hh"
#include "clp.h"
#include <tamer/fd.hh>

using tamer::event;
using namespace std;

namespace phat {

bool quiet = false;

tamed void handle_client(tamer::fd cfd) {
    tvars {
        msgpack_fd mpfd(cfd);
        Json req, res = Json::make_array();
    }

    while (cfd) {
        res.clear();
        twait { mpfd.read_request(make_event(req)); }
        if (!req) {
            if (!quiet)
                cerr << "client connection was closed: " << strerror(-cfd.error()) << endl;
            break;
        }
        else if (!req.is_a() || req.size() < 2 || !req[0].is_i()) {
            if (!quiet)
                cerr << "bad RPC: " << req << endl;
            break;
        }

        res[0] = -req[0].as_i();

        // look at rpc that was sent and take action

        mpfd.write(res);
    }

    cfd.close();
}

tamed void server(int32_t port) {
    tvars {
        tamer::fd sfd = tamer::tcp_listen(port);
        tamer::fd cfd;
    }
    if (sfd) {
        if (!quiet)
            cerr << "listening on port " << port << endl;
    }
    else {
        if (!quiet)
            cerr << "listen: " << strerror(-sfd.error()) << endl;
    }
    while (sfd) {
        twait { sfd.accept(make_event(cfd)); }
        handle_client(cfd);
    }
}

} //namespace


static Clp_Option options[] = {
    { "port", 'p', 0, Clp_ValInt, 0 },
    { "quiet", 'q', 0, 0, Clp_Negate }
};

int main(int argc, char** argv) {
    tamer::initialize();

    int32_t port = 17777;
    Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

    while (Clp_Next(clp) != Clp_Done) {
        if (Clp_IsLong(clp, "port"))
            port = clp->val.i;
        else if (Clp_IsLong(clp, "quiet"))
            phat::quiet = !clp->negated;
    }

    phat::server(port);

    tamer::loop();
    tamer::cleanup();
}