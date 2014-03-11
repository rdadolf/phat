.PHONY: default
.SECONDARY: ctl.cc ato.cc mpfd.hh

default: ctl

# All manner of FLAGS
TAMERC=../mprpc/tamer/compiler/tamer
TAMERFLAGS=-g
CXX=g++
CXXFLAGS=-g -std=gnu++0x -I.. -I../mprpc -I../mprpc/tamer -include config.h
LIBTAMER=../mprpc/tamer/tamer/.libs/libtamer.a
LIBS=$(LIBTAMER) `$(TAMERC) -l`
LDFLAGS=-L../mprpc/tamer -lrt -lpthread -lm $(LIBS)
MPRPC_HDR=../mprpc/msgpack.hh ../mprpc/mpfd.hh ../mprpc/string.hh ../mprpc/straccum.hh ../mprpc/json.hh ../mprpc/compiler.hh ../mprpc/clp.h
MPRPC_SRC=../mprpc/msgpack.cc ../mprpc/mpfd.cc ../mprpc/string.cc ../mprpc/straccum.cc ../mprpc/json.cc ../mprpc/compiler.cc ../mprpc/clp.c
MPRPC_OBJ=../mprpc/msgpack.o ../mprpc/mpfd.o ../mprpc/string.o ../mprpc/straccum.o ../mprpc/json.o ../mprpc/compiler.o ../mprpc/clp.o

# Build rules
controller: ctl
automaton: ato

ato: ato.cc $(MPRPC_HDR) $(MPRPC_SRC)
	$(CXX) $(CXXFLAGS) ato.cc $(MPRPC_SRC) -o ato $(LDFLAGS)

ctl: ctl.cc ../rpc_msg.hh $(MPRPC_HDR) $(MPRPC_SRC)
	$(CXX) $(CXXFLAGS) ctl.cc $(MPRPC_SRC) -o ctl $(LDFLAGS)

# Suffix rules for files that need TAMING
%.cc: %.tcc
	$(TAMERC) $(TAMERFLAGS) $< -o $@
%.hh: %.thh
	$(TAMERC) $(TAMERFLAGS) $< -o $@

# Cleanup
clean:
	rm -f ato.cc ctl.cc ato ctl
