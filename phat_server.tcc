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

Json* Phat_Server::traverse_files(Json& root, Json path) {
    Json* ret = &root;
    for (int i = 0; i < path.size(); ++i) {
        assert(path[i].is_s());
        if ((*ret)[path[i].as_s()] == Json::null) 
            return NULL;
        assert((*ret)[path[i].as_s()][0][1].is_i() 
                && (*ret)[path[i].as_s()][0][1].as_i() == DIR);
        Json& tmp = (*ret)[path[i].as_s()][1];
        ret = &tmp;
    }
    return ret;
}

Json Phat_Server::open(Json root, const char* subpath) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    (void) ret;
    return root;
}

Json Phat_Server::mkfile(Json root, const char* subpath, const char* data) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    if (ret)
        ret->set(name,Json::array(Json::array(name,FILE),data));
    return root;
}

Json Phat_Server::mkdir(Json root, const char* subpath) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    if(ret)
        ret->set(name,Json::array(Json::array(name,DIR),Json::make_object()));
    return root;
}

const char* Phat_Server::getcontents(Json root, const char* subpath) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    const char* data = NULL;
    if (ret && (*ret)[name]) {
        assert((*ret)[name][0][1].is_i() && (*ret)[name][0][1].as_i() == FILE);
        assert((*ret)[name][1].is_s());
        data = (*ret)[name][1].as_s().c_str();
    }
    return data;
}

Json Phat_Server::putcontents(Json root, const char* subpath, const char* data) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    if (ret && (*ret)[name]) {
        assert((*ret)[name][0][1].is_i() && (*ret)[name][0][1].as_i() == FILE);
        (*ret)[name][1] = data;
    }
    return root;
}

Json Phat_Server::readdir(Json root, const char* subpath) {
    Json path = parse_filepath(subpath);
    Json *p = traverse_files(root,path);
    Json ret = Json::make_array();
    if (p) {
        Json::object_iterator it = p->obegin();
        while (it) {
            Json tmp = Json::make_object();
            tmp.set(it->first,it->second[0]);
            ret.push_back(tmp);
            it++;
        }
    }
    return ret;
}

Json Phat_Server::stat(Json root, const char* subpath) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    if (ret && (*ret)[name]) {
        return (*ret)[name][0];
    }
    return Json::null;
}

void Phat_Server::remove(Json& root, const char* subpath) {
    Json path = parse_filepath(subpath);
    String name = path.back().as_s();
    path.pop_back();
    Json* ret = traverse_files(root,path);
    if (ret && (*ret)[name]) {
        // FIXME: check if file is open, by someone else?
        std::cout << "here: " << name <<std::endl;
        if ((*ret)[name][0][1].as_i() == DIR && !(*ret)[name][1].empty())
            return;
        ret->unset(name);
    }
    // FIXME: should we return something from this about whether it was successfully deleted?
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
