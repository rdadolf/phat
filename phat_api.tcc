#include <string>
#include <stdint.h>
#include "rpc_msg.hh"
#include "log.hh"
#include "network.hh"

#include "phat_api.hh"

using namespace phat;

Phat_Interface::Phat_Interface()
{
  Server_t default_contact_point;
  default_contact_point.port = DEFAULT_PHAT_PORT_;
  default_contact_point.ip.s_addr = htonl(INADDR_LOOPBACK);
  init(default_contact_point);
}

Phat_Interface::Phat_Interface(const Server_t contact_point)
{
  init(contact_point);
}

tamed void Phat_Interface::init(const Server_t contact_point)
{
  INFO() << "init called";

  // Assume contact point is master. They'll tell us otherwise.
  master_ = contact_point;
  replicas_.push_back(master_);
  epoch_number_ = -1; // A flag that we're new and also before any valid epoch.

  // Now ask our "master" who the master is.
  twait { get_master(make_event()); }

  // Now ask for replica list
  twait { get_replica_list(make_event()); } // FIXME: NYI

}

tamed void Phat_Interface::get_master(tamer::event<> ev)
{
  tvars {
    RPC_Msg request_master, reply_master;
    Json master_info;
    bool master_known=false;
    tamer::fd cfd;
  }

  INFO() << "get_master called";

  do {
    INFO() << "Attempting to contact master " << server_string(master_);
    twait { tamer::tcp_connect(master_.ip, master_.port, make_event(cfd)); }
    if( !cfd ) {
      ERROR() << "Couldn't connect to potential master at " << server_string(master_);
      exit(-1);
    }
    master_fd_.initialize(cfd); // Assume we have the true master.
    INFO() << "Potential master connected. Requesting master info.";
    request_master = RPC_Msg(Json::array(String("get_master")));
    twait { master_fd_.call(request_master, make_event(reply_master.json())); }
    if( !master_fd_ ) { // disconnected
      ERROR() << "Master candidate unexpectedly hungup. No contact points.";
      // FIXME: Do we really have no contacts here? If so, must that be true?
      exit(-1);
    }
// FIXME: validate
    // FIXME: validate
    if( reply_master.content()[0].as_s()=="ACK" ) {
      master_known = true;
      // master_ data already set.
    } else if( reply_master.content()[0].as_s()=="NACK" ) {
      master_known = false;
      if( reply_master.content()[1].as_s()=="NOT_MASTER" ) {
        if( reply_master.content()[2].is_s() ) {
          get_ip_address( reply_master.content()[2].as_s().c_str(), master_.ip );
        } else if( reply_master.content()[2].is_i() ) {
          get_ip_address( reply_master.content()[2].as_i(), master_.ip );
        }
        master_.port = reply_master.content()[3].as_i();
      }
      // FIXME: need handler for wrong epoch number reply
      // FIXME: need handler for arbitrary NACK (other than above)
    }
  } while( !master_known );

  INFO() << "Master identified";

  ev(); // FIXME
}

tamed void Phat_Interface::get_replica_list(tamer::event<> ev)
{
  ev(); // FIXME
}

////////////////////////////////////////////////////////////////////////////////

tamed void wait_for_notifications(tamer::event<Notification> ev)
{
  // FIXME Handle notifications
}

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

