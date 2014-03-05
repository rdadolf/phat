#include <stdio.h>
#include <iostream>
#include "mpfd.hh"

#define PORTNUM 15808

tamed void client()
{
  tvars {
    tamer::fd server_fd;
    msgpack_fd mpfd;
    Json msg = Json::array(String("getroot"));
    struct in_addr ip;
  }

  ip.s_addr = htonl(INADDR_LOOPBACK);
  twait {
    tamer::tcp_connect(ip,PORTNUM,make_event(server_fd));
  }
  if(!server_fd) {
    fprintf(stderr,"Couldn't connect: %s\n",strerror(-server_fd.error()));
    exit(-1);
  }
  mpfd.initialize(server_fd);
  while(1) {
    twait {
      at_delay(1, make_event());
      mpfd.write(msg);
    }
    std::cout << "request: " << msg << "\n";
  }
}

tamed void process(tamer::fd client_fd)
{
  tvars {
    msgpack_fd mpfd(client_fd);
    Json content;
  }
  while(client_fd) {
    twait { mpfd.read_request(make_event(content)); }
    std::cout << "process: " << content << "\n";
    //printf("%s\n", content[0].as_s().c_str());
  }
}

tamed void server()
{
  tvars {
    tamer::fd listen_fd = tamer::tcp_listen(PORTNUM);
    tamer::fd client_fd;
  }
  if (!listen_fd) {
    fprintf(stderr,"Error from tamer::tcp_listen: %s\n", strerror(-listen_fd.error()));
    // Exit?
  }
  while(listen_fd) {
    twait { listen_fd.accept(make_event(client_fd)); }
    process(client_fd); // One client at a time.
  }
}


int main(int argc, char **argv)
{
  char mode;

  if(argc<2) {
    fprintf(stderr,"Usage: %s <c/s>\n", argv[0]);
    return -1;
  }

  tamer::initialize();

  mode = argv[1][0];

  if (mode=='c') {
    client();
  } else if(mode=='s') {
    server();
  }

  tamer::loop();
  tamer::cleanup();
  return 0;
}
