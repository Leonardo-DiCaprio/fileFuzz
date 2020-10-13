#ifndef MEDIAINPUTENCODER_H_
#define MEDIAINPUTENCODER_H_

#include "mediaEncoder.h"

#include "streamConfig.h"
#include "oggPacket.h"
#include "oggComment.h"

class MediaInputEncoder : public MediaEncoder {

protected:
  uint8   streamNo;

public:
  MediaInputEncoder(const uint8 streamNo);
  virtual ~MediaInputEncoder();

  virtual MediaInputEncoder& operator>>(OggPacket& packet) = 0;

  virtual void configureEncoder(StreamConfig& config, std::vector<OggComment>& oggComments) = 0;

  uint32 getStreamNo() const;
};

#endif /*MEDIAINPUTENCODER_H_*/
