.PHONY: default test

CPP_TEMPS= client_driver.cc paxos.cc paxos_test.cc phat_api.cc phat_server.cc puppet.cc server_driver.cc
HPP_TEMPS= paxos.hh phat_api.hh phat_server.hh puppet.hh
.SECONDARY:$(CPP_TEMPS) $(HPP_TEMPS)

default: server_driver client_driver puppet

# All manner of FLAGS
OS_STYLE=$(shell uname -s)
OS_MAJOR=$(shell uname -r | sed -e 's/\..*//')
OS_MINOR=$(shell uname -r | sed -e 's/[^.]*\.\([^.]*\).*/\1/')
OS_VERSION=$(shell uname -r | perl -ne '/(\d+\.\d+)/; print 0+$$1')

OS=UNKNOWN

ifeq ($(OS_STYLE), Linux)
OS=LINUX
endif

ifeq ($(OS_STYLE),Darwin)
ifeq ($(shell perl -e 'print int($(OS_MAJOR)>=13 && $(OS_MINOR)>=0)'), 1)
OS=OSX_POSTMAV
else
OS=OSX_PREMAV
endif
endif


DEBUG=-g -DDEBUG=1

TAMERC=mprpc/tamer/compiler/tamer
TAMERFLAGS=
#TAMERFLAGS=-n -L
ifeq ($(OS),LINUX)
CXX=g++ -std=gnu++0x
endif
ifeq ($(OS),OSX_POSTMAV)
CXX=g++ -std=c++11
#CXX=g++ -std=c++11 -stdlib=libc++
endif
CXXFLAGS=-Wall -g -O2 $(DEBUG) -I. -Imprpc -Imprpc/tamer -Imprpc/.deps -include config.h
LIBTAMER=mprpc/tamer/tamer/.libs/libtamer.a
LIBS=$(LIBTAMER) `$(TAMERC) -l`
LDFLAGS= -lpthread -lm $(LIBS)
MPRPC_SRC=mprpc/msgpack.cc mprpc/.deps/mpfd.cc mprpc/string.cc mprpc/straccum.cc mprpc/json.cc mprpc/compiler.cc mprpc/clp.c
MPRPC_OBJ=mprpc/msgpack.o mprpc/mpfd.o mprpc/string.o mprpc/straccum.o mprpc/json.o mprpc/compiler.o mprpc/clp.c
MPRPC_HDR=mprpc/msgpack.hh mprpc/.deps/mpfd.hh mprpc/string.hh mprpc/straccum.hh mprpc/json.hh mprpc/compiler.hh mprpc/clp.h

UTIL_OBJ=network.o

CLIENT_HDR=phat_api.hh puppet.hh rpc_msg.hh log.hh network.hh
SERVER_HDR=phat_server.hh puppet.hh rpc_msg.hh log.hh network.hh

# Test scripts
clean_puppet:
	rm -f puppet
simple: clean_puppet
	make TEST_SCRIPT=test/simple.hh
die: clean_puppet
	make TEST_SCRIPT=test/die.hh
rw: clean_puppet
	make TEST_SCRIPT=test/rw.hh
getroot: clean_puppet
	make TEST_SCRIPT=test/getroot.hh
mkfile: clean_puppet
	make TEST_SCRIPT=test/mkfile.hh
apitest: clean_puppet
	make TEST_SCRIPT=test/apitest.hh
paxos_test: clean_puppet
	make TEST_SCRIPT=test/paxos_test.hh

# Build rules
client_driver.o: client_driver.cc $(CLIENT_HDR)
phat_api.o: phat_api.cc phat_api.hh $(CLIENT_HDR)
client_driver: client_driver.o phat_api.o $(UTIL_OBJ) $(MPRPC_OBJ) $(MPRPC_SRC) $(MPRPC_HDR)
	$(CXX) client_driver.o phat_api.o $(MPRPC_OBJ) $(UTIL_OBJ) -o client_driver $(LDFLAGS)

server_driver.o: server_driver.cc $(SERVER_HDR)
phat_server.o: phat_server.cc $(SERVER_HDR)
server_driver: server_driver.o phat_server.o $(UTIL_OBJ) $(MPRPC_OBJ) $(MPRPC_SRC) $(MPRPC_HDR)
	$(CXX) server_driver.o phat_server.o $(UTIL_OBJ) $(MPRPC_OBJ) -o server_driver $(LDFLAGS)

TEST_SCRIPT ?= test/simple.hh
puppet: puppet.o puppet.hh rpc_msg.hh $(UTIL_OBJ) $(MPRPC_HDR) $(MPRPC_SRC) $(TEST_SCRIPT)
	$(CXX) $(CXXFLAGS) -DTEST_SCRIPT='"$(TEST_SCRIPT)"' puppet.cc $(UTIL_OBJ) $(MPRPC_SRC) -o puppet $(LDFLAGS)

# Suffix rules for files that need TAMING
%.cc: %.tcc
	$(TAMERC) $(TAMERFLAGS) -o $@ $<
%.cc: test/%.tcc
	$(TAMERC) $(TAMERFLAGS) -o $@ $<
%.hh: %.thh
	$(TAMERC) $(TAMERFLAGS) -o $@ $<

paxos.o: $(MPRPC_HDR) $(MPRPC_OBJ) $(MPRPC_SRC) paxos.hh paxos.cc
paxos_test.o: paxos_test.cc paxos.hh mprpc/.deps/mpfd.hh
ex/log_test.o: ex/log_test.cc

# Cleanup
clean:
	rm -f client_driver server_driver puppet paxos_test *.o
	rm -f $(CPP_TEMPS)
	rm -f $(HPP_TEMPS)
