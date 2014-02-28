# placeholder

TAMERC=mprpc/tamer/compiler/tamer

default: phat.cc phat.hh


phat.cc: %.cc: %.tcc
	$(TAMERC) -c $<

phat.hh: %.hh: %.thh
	$(TAMERC) -c $<
