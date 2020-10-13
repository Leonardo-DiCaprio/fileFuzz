#ifndef THEORAENCODER_H_
#define THEORAENCODER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#warning only use with autotools
#endif

#ifdef HAVE_LIBTHEORAENC

#include <list>
#include <vector>
#include <theora/codec.h>
#include <theora/theoraenc.h>

#include "oggPacket.h"
#include "oggComment.h"
#include "mediaInputEncoder.h"
#include "theoraStreamParameter.h"
#include "streamConfig.h"

class TheoraEncoder : public MediaInputEncoder {

protected:

  uint64               packetCounter;

  th_enc_ctx*          theoraState;
  th_comment           theoraComment;
  th_info              theoraInfo;

  std::list<OggPacket> packetList;

  void createHeader(std::vector<OggPacket>& headerList, std::vector<OggComment>& oggComments);

public:

  TheoraEncoder(uint8 streamNo = 0);
  virtual ~TheoraEncoder();

  virtual MediaInputEncoder& operator>>(OggPacket& packet);
  MediaInputEncoder& operator<<(th_ycbcr_buffer buffer);

  virtual void configureEncoder(StreamConfig& config, std::vector<OggComment>& oggComments);

  uint32 width() const;
  uint32 height() const;

  th_info&   getInfo();

  virtual std::string configuration() const;

  virtual void reset();

};

#endif /* HAVE_LIBTHEORAENC */

#endif /*THEORAENCODER_H_*/
