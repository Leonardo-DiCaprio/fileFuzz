#ifndef vorbisDecoder_h
#define vorbisDecoder_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBVORBIS

#include <list>
#include <vector>
#include <vorbis/codec.h>
#include "definition.h"

#include "mediaOutputDecoder.h"
#include "audioPacket.h"

class VorbisDecoder : public MediaOutputDecoder {

protected:

  vorbis_info      vorbisInfo;
  vorbis_dsp_state vorbisDspState;
  vorbis_block     vorbisBlock;
  vorbis_comment   vorbisComment;

  std::list<AudioPacket>  packetList;

  uint8   initCount;
  uint32  packetCount;

  uint64  sampleCounter;

public:

  VorbisDecoder(uint8 streamID = 0);
  virtual ~VorbisDecoder();

  virtual void initDecoder(StreamConfig& config, std::vector<OggComment>& oggComments);

//  virtual std::string getInfoString();

  vorbis_info&        getInfo();
  vorbis_comment&     getComment();

  virtual MediaOutputDecoder& operator<<(OggPacket packet);
  VorbisDecoder& operator>>(AudioPacket& audioPacket);

  virtual void clear();

  virtual double getTimeOfNextPacket();
  virtual uint32 getPositionOfNextPacket() {
    return packetCount;
  }

  virtual std::string configuration() const;


};

#endif // WITH_LIBVORBIS

#endif

