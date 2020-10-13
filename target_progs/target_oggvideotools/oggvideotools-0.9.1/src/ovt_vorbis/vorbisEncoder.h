#ifndef VORBISENCODER_H_
#define VORBISENCODER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBVORBIS

#include <list>
#include <vector>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

#include "mediaInputEncoder.h"
#include "oggPacket.h"
#include "audioPacket.h"
#include "oggComment.h"
#include "vorbisStreamParameter.h"
#include "streamConfig.h"

class VorbisEncoder : public MediaInputEncoder {
protected:

  vorbis_info vorbisInfo;
  vorbis_comment vorbisComment;
  vorbis_dsp_state vorbisState;
  vorbis_block vorbisBlock;

//  OggPacket packet;

  std::list<OggPacket> packetList;

  uint64 pktCnt;

public:
  VorbisEncoder(uint8 streamNo);
  virtual ~VorbisEncoder();

  virtual MediaInputEncoder& operator>>(OggPacket& packet);
  MediaInputEncoder& operator<<(AudioPacket& aPacket);

  virtual void configureEncoder(StreamConfig& streamConf, std::vector<OggComment>& oggComments);
  void flush();

  vorbis_info& getInfo();

  virtual std::string configuration() const;

};

#endif /* HAVE_LIBVORBIS */
#endif /* VORBISENCODER_H_*/

