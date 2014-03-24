#include <stdio.h>
#include "mpfd.hh"
#include "json.hh"
#include <netdb.h>
#include "clp.h"
#include "rpc_msg.hh"
#include "log.hh"
#include "network.hh"
#include "filepath.hh"

#include "phat_server.hh"

using namespace phat;

Phat_Server::Phat_Server()
{
  listen_port_ = 15810;
  run_master_server();
}

Phat_Server::Phat_Server(int port)
{
  listen_port_ = port;
  run_master_server();
}

void Phat_Server::run_master_server()
{
  INFO() << "Running phat master server";
  handle_new_connections(listen_port_);
}

void Phat_Server::run_replica_server()
{
  ; // FIXME: NYI
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

  INFO() << "Phat server listening on " << port;

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
    INFO() << "Received RPC: " << request.json();
    if(!mpfd)
      break;
    if( request.validate() ) {
      if( request.content()[0]=="get_master" ) {
        reply = RPC_Msg( get_master(request.content()), request );
      } else if( request.content()[0]=="get_replica_list" ) {
        reply = RPC_Msg(Json::array(String("NACK"),String("NYI")), request);
      } else if( request.content()[0]=="getroot" ) {
        reply = RPC_Msg( getroot(request.content()), request );
      //} else if( request.content()[0]=="<other thing here>" ) {
      } else {
        reply = RPC_Msg( Json::array(String("NACK")), request );
      }
    } else {
      reply = RPC_Msg( Json::array(String("NACK")), request );
    }
    INFO() << "Writing reply: " << reply.json();
    mpfd.write(reply);
  }
}

////////////////////////////////////////////////////////////////////////////////

Json Phat_Server::getroot(Json args)
{
  return Json::array(String("/"));
}

Json Phat_Server::get_master(Json args)
{
  INFO() << "get_master service routine called";
  if( i_am_master_ ) {
    return Json::array(String("ACK"));
  } else {
    // FIXME: Report real master.
    return Json::array(String("NACK"),String("NOT_MASTER"),-1,-1);
  }
}

Json Phat_Server::get_replica_list(Json args)
{
  return Json::null;
}
