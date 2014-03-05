#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"

#include "puppet.hh"

#ifndef TEST_SCRIPT
tamed void run()
{
  printf("No test script specified. Exiting.\n");
}
#else
  #include TEST_SCRIPT
#endif

int main(int argc, char **argv)
{
  tamer::initialize();

  run();
  tamer::loop();
  tamer::cleanup();
  return 0;
}
