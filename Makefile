.PHONY: default

CPP_TEMPS= client_driver.cc paxos.cc paxos.hh paxos_test.cc phat_api.cc phat_api.hh phat_server.cc puppet.cc puppet.hh server_driver.cc
.SECONDARY:$(CPP_TEMPS)

default: server_driver client_driver puppet

# Test scripts
simple:
	make TEST_SCRIPT=test/simple.hh
die:
	make TEST_SCRIPT=test/die.hh
rw:
	make TEST_SCRIPT=test/rw.hh


# All manner of FLAGS
DEBUG=-g -DDEBUG=1

TAMERC=mprpc/tamer/compiler/tamer
TAMERFLAGS=-n
CXX=g++
CXXFLAGS=-Wall $(DEBUG) -std=gnu++0x -I. -Imprpc -Imprpc/tamer -Imprpc/.deps -include config.h
LIBTAMER=mprpc/tamer/tamer/.libs/libtamer.a
LIBS=$(LIBTAMER) `$(TAMERC) -l`
LDFLAGS=-L../mprpc/tamer -lrt -lpthread -lm $(LIBS)
MPRPC_SRC=mprpc/msgpack.cc mprpc/.deps/mpfd.cc mprpc/string.cc mprpc/straccum.cc mprpc/json.cc mprpc/compiler.cc mprpc/clp.c
MPRPC_OBJ=mprpc/msgpack.o mprpc/mpfd.o mprpc/string.o mprpc/straccum.o mprpc/json.o mprpc/compiler.o mprpc/clp.c
MPRPC_HDR=mprpc/msgpack.hh mprpc/.deps/mpfd.hh mprpc/string.hh mprpc/straccum.hh mprpc/json.hh mprpc/compiler.hh mprpc/clp.h

# Build rules
client_driver.o: client_driver.cc phat_api.hh puppet.hh 
phat_api.o: phat_api.hh
client_driver: client_driver.o phat_api.o $(MPRPC_OBJ) $(MPRPC_SRC) $(MPRPC_HDR)
	$(CXX) client_driver.o phat_api.o $(MPRPC_OBJ) -o client_driver $(LDFLAGS)

server_driver.o: server_driver.cc puppet.hh
server_driver: server_driver.o $(MPRPC_OBJ) $(MPRPC_SRC) $(MPRPC_HDR)
	$(CXX) server_driver.o $(MPRPC_OBJ) -o server_driver $(LDFLAGS)

TEST_SCRIPT ?= test/simple.hh
puppet: puppet.o puppet.hh $(MPRPC_HDR) $(MPRPC_SRC) $(TEST_SCRIPT)
	$(CXX) $(CXXFLAGS) -DTEST_SCRIPT='"$(TEST_SCRIPT)"' puppet.cc $(MPRPC_SRC) -o puppet $(LDFLAGS)

paxos_test: paxos_test.o paxos.o $(MPRPC_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS)
	
# Suffix rules for files that need TAMING
%.cc: %.tcc
	$(TAMERC) $(TAMERFLAGS) $< -o $@
%.cc: test/%.tcc
	$(TAMERC) $(TAMERFLAGS) $< -o $@
%.hh: %.thh
	$(TAMERC) $(TAMERFLAGS) $< -o $@

paxos.o: $(MPRPC_HDR) $(MPRPC_OBJ) $(MPRPC_SRC) paxos.hh paxos.cc
paxos_test.o: paxos_test.cc paxos.hh mprpc/.deps/mpfd.hh
ex/log_test.o: ex/log_test.cc

# Cleanup
clean:
	rm -f client_driver server_driver puppet paxos_test *.o
	rm -f $(CPP_TEMPS)
