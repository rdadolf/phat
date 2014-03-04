#include "phat.hh"
#include "clp.h"

namespace phat {

bool quiet = false;

tamed void do_test(String host, uint32_t port) {
    tvars {
        Phat phat;
    }

    twait { phat.connect(host, port, make_event()); }

    twait { phat.disconnect(make_event()); }
}

} // namespace


static Clp_Option options[] = {
    { "host", 'h', 0, Clp_ValString, 0 },
    { "port", 'p', 0, Clp_ValInt, 0 },
    { "quiet", 'q', 0, 0, Clp_Negate }
};

int main(int argc, char** argv) {
    tamer::initialize();

    String host = "localhost";
    uint32_t port = 17777;
    Clp_Parser* clp = Clp_NewParser(argc, argv, sizeof(options) / sizeof(options[0]), options);

    while (Clp_Next(clp) != Clp_Done) {
        if (Clp_IsLong(clp, "host"))
            host = clp->vstr;
        else if (Clp_IsLong(clp, "port"))
            port = clp->val.i;
        else if (Clp_IsLong(clp, "quiet"))
            phat::quiet = !clp->negated;
    }

    phat::do_test(host, port);

    tamer::loop();
    tamer::cleanup();
}
