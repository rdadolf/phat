#include <tamer/tamer.hh>
#include <iostream>
#include "log.hh"

class test {
public:
    test();
    test(const char* name,int port) {}
    tamed void test_log(tamer::event<> done);
    tamed void test_log2(tamer::event<> done);
    void test2(void) {
        INFO() << "test2";
    }
};

tamed void test::test_log(tamer::event<> done) {
    INFO() << "test";
    done();
}

tamed void test::test_log2(tamer::event<> done) {
    test2();
    done();
}

tamed void test_log(void) {
    tvars { 
        test t("Proposer",18029);
        tamer::rendezvous<> r;
    }
    tamer::at_delay_sec(1,r.make_event());
    twait(r);
    twait { t.test_log(make_event()); }
    std::cout << "test1 done" << std::endl;
    twait { t.test_log2(make_event()); }
    std::cout << "test2 done" << std::endl;
}


int main () {
    tamer::initialize();

    test_log();

    tamer::loop();
    tamer::cleanup();
}
