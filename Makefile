.PHONY: default client server

default: client server

# All manner of FLAGS
TAMERC=mprpc/tamer/compiler/tamer
CXX=g++
CXXFLAGS=-Imprpc
LDFLAGS=

# Build rules
server: phat
phat: phat.cc phat.hh paxos.cc

client: api.hh

# Suffix rules for files that need TAMING
%.cc: %.tcc
	$(TAMERC) -c $<
%.hh: %.thh
	$(TAMERC) -c $<

# Cleanup
clean:
	rm -f phat.cc phat.hh paxos.cc phat
