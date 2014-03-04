#include "phat.hh"

using tamer::event;
using std::vector;

namespace phat {

Phat::Phat() : mpfd_(nullptr) {
}

tamed void Phat::connect(String host, uint32_t port, event<> done) {
    assert(!mpfd_);
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
