#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"
#include "rpc_msg.hh"

#include "phat_server.hh"

using namespace phat;

Phat_Server::Phat_Server()
{
  run_master_server(); // FIXME
}

Phat_Server::Phat_Server(int port)
{
  listen_port_ = port;
  run_master_server(); // FIXME
}

void Phat_Server::run_master_server()
{
  handle_new_connections(listen_port_);
}

void Phat_Server::run_replica_server()
{

}

////////////////////////////////////////////////////////////////////////////////

tamed void Phat_Server::handle_new_connections(int port)
{
  tvars {
    tamer::fd client_fd; // FIXME: only one simultaneous client. :(
    tamer::fd listen_fd;
  }
  
  listen_fd = tamer::tcp_listen(port);
  if( !listen_fd ) {
    fprintf(stderr, "Phat server unable to listen on %d: %s.\n", port, strerror(-listen_fd.error()));
    exit(-1);
  }

  while(listen_fd) {
    twait {
      listen_fd.accept(make_event(client_fd));
    }
    read_and_dispatch(client_fd);
  }
}

tamed void Phat_Server::read_and_dispatch(tamer::fd client_fd)
{
  tvars {
    msgpack_fd mpfd;
    RPC_Msg request, reply;
  }

  mpfd.initialize(client_fd);
  while(mpfd) {
    twait{ mpfd.read_request(tamer::make_event(request.json())); }
    if( request.validate() ) {
      if( request.content()[0]=="get_master" ) {
        if( i_am_master_ ) {
          reply = RPC_Msg(Json::array(String("ACK")));
        } else {
          // FIXME: Report real master.
          reply = RPC_Msg(Json::array(String("NACK"),String("NOT_MASTER"),-1,-1));
        }
      } else if( request.content()[0]=="get_replica_list" ) {
        reply = RPC_Msg(Json::array(String("NACK"),String("NYI")));
      } else if( request.content()[0]=="getroot" ) {
        reply = RPC_Msg( getroot(request.content()), request );
      //} else if( request.content()[0]=="<other thing here>" ) {
      } else {
        reply = RPC_Msg( Json::array(String("NACK")), request );
      }
    } else {
      reply = RPC_Msg( Json::array(String("NACK")), request );
    }
    mpfd.write(reply);
  }
}

////////////////////////////////////////////////////////////////////////////////

Json Phat_Server::getroot(Json args)
{
  return Json::array(String("/"));
}

