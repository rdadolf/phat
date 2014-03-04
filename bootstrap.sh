#! /bin/sh

if [ ! -f mprpc/boostrap.sh ]; then
    git submodule init
    git submodule update
fi

( cd mprpc; ./bootstrap.sh )

autoreconf -i
