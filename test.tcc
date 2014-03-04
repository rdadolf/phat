#include "phat.hh"


int main(int argc, char **argv) {
    tamer::initialize();

    tamer::loop();
    tamer::cleanup();
    return 0;
}
