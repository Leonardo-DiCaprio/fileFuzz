#include "bufferRepository.h"

BufferRepository::BufferRepository(const std::string& name)
  : MediaRepository(MediaUnit::readwrite, name)
{
}

BufferRepository::~BufferRepository()
{
}

MediaUnit& BufferRepository::operator<<(RawMediaPacket& packet)
{
  buffer.push_back(packet);

  return(*this);
}

MediaUnit& BufferRepository::operator>>(RawMediaPacket& packet)
{
  if (!buffer.empty()) {
    packet = buffer.front();
    buffer.pop_front();
  }

  return(*this);
}

bool BufferRepository::isAvailable()
{
  return(!buffer.empty());
}

uint32 BufferRepository::getBunchSize()
{
  return(0);
}

void BufferRepository::setBunchSize(uint32 size)
{
  return;
}
