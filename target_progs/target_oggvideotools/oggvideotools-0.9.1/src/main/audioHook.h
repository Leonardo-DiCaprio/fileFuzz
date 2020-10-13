#ifndef AUDIOHOOK_H_
#define AUDIOHOOK_H_

#include "hookHandler.h"
#include "audioConverter.h"
#include "audioPacket.h"

#include "vorbisDecoder.h"
#include "vorbisEncoder.h"

class AudioHook : public HookHandler {

private:
  bool changeAudioSamplerate;
  bool changeChannel;
  bool copy;

  AudioConverter converter;
  AudioPacket    audioPacket;

  AudioHook();

public:
  AudioHook(uint8 outStreamID, const bool copy, const bool keepComments);
  virtual ~AudioHook();

  virtual HookHandler& operator<<(OggPacket& packet);

  virtual void initAndConnect();

  virtual void flush();

  virtual OggType getType() const;
};

static bool operator==(const vorbis_info& info1, const vorbis_info& info2);
static bool operator!=(const vorbis_info& info1, const vorbis_info& info2);


#endif /*AUDIOHOOK_H_*/
