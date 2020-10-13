#ifndef MEDIAOUTPUTDECODER_H_
#define MEDIAOUTPUTDECODER_H_

#include "definition.h"

#include "oggPacket.h"
#include "oggComment.h"
#include "mediaDecoder.h"
#include "streamConfig.h"

class MediaOutputDecoder : public MediaDecoder {

protected:
  uint8             streamID;

public:
  MediaOutputDecoder(const uint8 _streamID = 0);
  virtual ~MediaOutputDecoder();

  virtual void initDecoder(StreamConfig& config, std::vector<OggComment>& commentList) = 0;
  virtual MediaOutputDecoder& operator<<(OggPacket packet) = 0;

  virtual uint32 getPositionOfNextPacket() = 0;

  uint8    getStreamNo() const;

};

#endif /*MEDIAOUTPUTDECODER_H_*/
