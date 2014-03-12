#include <stdio.h>
#include <iostream>
#include "mpfd.hh"

#include "rpc_msg.hh"

#define PORTNUM 15808

// FIXME Enable buggy behavior.
#define RPC_MSG_ISSUE true

tamed void client()
{
  tvars {
    tamer::fd server_fd;
    msgpack_fd mpfd;
    RPC_Msg request = RPC_Msg(Json::array(String("getroot")));
    Json reply;
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
    std::cout << "sending: " << request.content() << "\n";
    twait {
      at_delay(1, make_event());
      mpfd.call(request,make_event(reply));
    }
    std::cout << "received: " << reply << "\n";
  }
}

tamed void process(msgpack_fd &mpfd)
{
  tvars {
    Json request, reply;
    RPC_Msg rpc_request, rpc_reply;
  }
  while(mpfd) {
    twait { 
    #ifdef RPC_MSG_ISSUE
    mpfd.read_request(tamer::make_event(rpc_request.json()));
    #else
    mpfd.read_request(tamer::make_event(request.json()));
    #endif
    }
    #ifdef RPC_MSG_ISSUE
    std::cout << "received: " << rpc_request.content() << "\n";
    rpc_reply = RPC_Msg(Json::array("ACK"),request);
    std::cout << "replying: " << rpc_reply.content() << "\n";
    mpfd.write(rpc_reply);
    #else
    std::cout << "received: " << request << "\n";
    reply = Json::array(-1,request[1].as_i(),Json::array("ACK"));
    std::cout << "replying: " << reply << "\n";
    mpfd.write(reply);
    #endif
    //printf("%s\n", content[0].as_s().c_str());
  }
}

tamed void server()
{
  tvars {
    tamer::fd listen_fd = tamer::tcp_listen(PORTNUM);
    tamer::fd client_fd;
    msgpack_fd mpfd;
  }
  if (!listen_fd) {
    fprintf(stderr,"Error from tamer::tcp_listen: %s\n", strerror(-listen_fd.error()));
    // Exit?
  }
  while(listen_fd) {
    twait { listen_fd.accept(make_event(client_fd)); }
    mpfd.initialize(client_fd);
    process(mpfd); // One client at a time.
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
