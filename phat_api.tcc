#include <string>
#include <stdint.h>
#include "rpc_msg.hh"

#include "phat_api.hh"

using namespace phat;

Phat_Interface::Phat_Interface()
{
  Server_t default_master;
  default_master.port = DEFAULT_PHAT_PORT_;
  default_master.ip.s_addr = INADDR_LOOPBACK;
  connect_to_master(default_master);
}

Phat_Interface::Phat_Interface(const Server_t contact_point)
{
  connect_to_master(contact_point);
}

tamed void Phat_Interface::connect_to_master(const Server_t contact_point)
{
  tvars {
    tamer::fd contact_fd;
    bool known_master = false;
    RPC_Msg master_json, replicas_json;
  }

  // Make contact with *some* server.
  master_ = contact_point;
  twait{ tamer::tcp_connect(master_.ip, master_.port, make_event(contact_fd)); }
  if( !contact_fd ) {
    fprintf(stderr,"Couldn't connect to contact point %d on port %d: %s\n", master_.ip.s_addr, master_.port, strerror(-contact_fd.error()));
    exit(-1);
  }
  master_fd_.initialize(contact_fd);

  while( !known_master ) {
    twait{ get_master(make_event(master_json)); }
    // FIXME: more content validation
    if( master_json.content().is_a()
     && (master_json.content()[0].is_i() || master_json.content()[0].is_s()) // in_addr or hostname
     && master_json.content()[1].is_i() // port number
      ) {
      if( master_json.content()[0].is_s() ) {
        // FIXME: hostname lookup
      } else {
        master_.ip.s_addr = master_json.content()[0].as_i();
      }
      master_.port = master_json.content()[1].as_i();
    } else {
      fprintf(stderr,"Received invalid master contact info from server.\n");
      exit(-1);
    }
  }

  // FIXME: Do this!
  // twait{ get_replica_list(make_event(replicas_json)); }
}

tamed void Phat_Interface::get_master(tamer::event<RPC_Msg> ev)
{
  ev(RPC_Msg(Json::null)); // FIXME
}

tamed void Phat_Interface::get_replica_list(tamer::event<RPC_Msg> ev)
{
  ev(RPC_Msg(Json::null)); // FIXME
}

////////////////////////////////////////////////////////////////////////////////

Handle Phat_Interface::getroot()
{
  return Handle();
}

Handle Phat_Interface::open(Handle root, const std::string subpath)
{

  return Handle();
}

Handle Phat_Interface::mkfile(Handle root, const std::string subpath, const char *data)
{

  return Handle();
}

Handle Phat_Interface::mkdir(Handle root, const std::string subpath)
{

  return Handle();
}

const char *Phat_Interface::getcontents(Handle h)
{
  return NULL;
}

void Phat_Interface::putcontents(Handle h, const char *data)
{
  return;
}

std::string Phat_Interface::readdir(Handle h)
{
  return "";
}

Metadata Phat_Interface::stat(Handle f)
{
  return Metadata();
}

Sequencer Phat_Interface::flock(Handle f, Locktype lt)
{
  return Sequencer();
}

Sequencer Phat_Interface::funlock(Handle f)
{
  return Sequencer();
}

void Phat_Interface::remove(Handle f)
{
  return;
}

