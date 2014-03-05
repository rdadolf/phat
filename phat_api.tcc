#include <string>
#include <stdint.h>

#include "phat_api.hh"

using namespace phat;

Phat_API::Phat_API()
{

}

Handle Phat_API::getroot()
{
  return Handle();
}

Handle Phat_API::open(Handle root, const std::string subpath)
{

  return Handle();
}

Handle Phat_API::mkfile(Handle root, const std::string subpath, const char *data)
{

  return Handle();
}

Handle Phat_API::mkdir(Handle root, const std::string subpath)
{

  return Handle();
}

const char *Phat_API::getcontents(Handle h)
{
  return NULL;
}

void Phat_API::putcontents(Handle h, const char *data)
{
  return;
}

std::string Phat_API::readdir(Handle h)
{
  return "";
}

Metadata Phat_API::stat(Handle f)
{
  return Metadata();
}

Sequencer Phat_API::flock(Handle f, Locktype lt)
{
  return Sequencer();
}

Sequencer Phat_API::funlock(Handle f)
{
  return Sequencer();
}

void Phat_API::remove(Handle f)
{
  return;
}

