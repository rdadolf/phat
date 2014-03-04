.PHONY: default server
.SECONDARY: phat.cc paxos.cc client.cc puppet.cc

default: server client puppet

# All manner of FLAGS
TAMERC=../mprpc/tamer/compiler/tamer
TAMERFLAGS=-g
CXX=g++
CXXFLAGS=-g -std=gnu++0x -Imprpc -Imprpc/tamer -include config.h
LIBTAMER=../mprpc/tamer/tamer/.libs/libtamer.a
LIBS=$(LIBTAMER) `$(TAMERC) -l`
LDFLAGS=-L../mprpc/tamer -lrt -lpthread -lm $(LIBS)
MPRPC=mprpc

# Build rules
server: phat

phat: phat.cc puppet.hh paxos.cc $(MPRPC)
	$(CXX) $(CXXFLAGS) phat.cc paxos.cc mprpc/clp.c -o phat $(LDFLAGS)

client: client.cc puppet.hh api.hh $(MPRPC)
	$(CXX) $(CXXFLAGS) client.cc mprpc/clp.c -o client $(LDFLAGS)

puppet: puppet.cc puppet.hh $(MPRPC)
	$(CXX) $(CXXFLAGS) puppet.cc mprpc/clp.c -o puppet $(LDFLAGS)

# Library dependencies
mprpc:
	make -C mprpc

# Suffix rules for files that need TAMING
%.cc: %.tcc
	$(TAMERC) $(TAMERFLAGS) $< -o $@
%.hh: %.thh
	$(TAMERC) $(TAMERFLAGS) $< -o $@

# Cleanup
clean:
	rm -f phat phat.cc paxos.cc client client.cc api.hh puppet puppet.cc puppet.hh
