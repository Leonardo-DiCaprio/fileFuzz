#ifndef BUFFERREPOSITORY_H_
#define BUFFERREPOSITORY_H_

#include <deque>

#include "mediaRepository.h"

class BufferRepository : public MediaRepository {
  std::deque<RawMediaPacket> buffer;

public:
  BufferRepository(const std::string& name = std::string("BufferRepository"));
  virtual ~BufferRepository();

  virtual MediaUnit& operator<<(RawMediaPacket& packet);
  virtual MediaUnit& operator>>(RawMediaPacket& packet);

  virtual bool isAvailable();

  virtual uint32 getBunchSize();

  virtual void setBunchSize(uint32 size);

};

#endif /*BUFFERREPOSITORY_H_*/
