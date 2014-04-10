# Phat/Joan

An implementation of the Chubby lock service protocol in MPRPC. Named Joan.

## Installation

    git clone <phat>
    cd phat
    git submodule init
    git submodule update
    cd mprpc
    ./bootstrap.sh
    ./configure CXX='g++ -std=gnu++0x'
    make
    cd ..
    make

## Running tests

Tests are run using the `puppet` program. To run a test, first build a version of `puppet` which incorporates that test:

    make TEST_SCRIPT=test/<filename>.thh

This creates an executable which will automatically orchestrate the test.

