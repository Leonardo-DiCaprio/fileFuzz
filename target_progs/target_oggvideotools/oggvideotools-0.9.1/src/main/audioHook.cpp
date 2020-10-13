#include "audioHook.h"
#include <iostream>
#include "exception.h"

#include "vorbisEncoder.h"
#include "vorbisDecoder.h"
#include "log.h"

AudioHook::AudioHook()
{}

AudioHook::AudioHook(uint8 outStreamID, const bool copy, const bool keepComments)
  : HookHandler(copy, keepComments), changeAudioSamplerate(false), changeChannel(false)
{
//	logger.debug() << "Vorbis Encoder stream No "<<(int)outStreamID<<std::endl;

  outputDecoder.reset(new VorbisDecoder);
  inputEncoder.reset(new VorbisEncoder(outStreamID));

}

AudioHook::~AudioHook()
{
  converter.closeResample();
}

void AudioHook::initAndConnect()
{
  VorbisDecoder& decoder = static_cast<VorbisDecoder&>(*outputDecoder.get());
  VorbisEncoder& encoder = static_cast<VorbisEncoder&>(*inputEncoder.get());

  copy = true;

  if (!copyOnly)
    copy = decoder.getInfo() == encoder.getInfo();

  if (!copy) {
    if (decoder.getInfo().channels != encoder.getInfo().channels)
      changeChannel = true;

    if (decoder.getInfo().rate != encoder.getInfo().rate)
      changeAudioSamplerate = true;

  }

  converter.closeResample();

  converter.initResample(encoder.getInfo().channels,
                         (encoder.getInfo().rate * 1.0 )
                         / (decoder.getInfo().rate * 1.0 ));

}

static uint64 cnt(0);

HookHandler& AudioHook::operator<<(OggPacket& packet)
{
  if (!outputDecoder)
    throw OggException("AudioHook::callHook: no outputDecoder given");

  if (!inputEncoder)
    throw OggException("AudioHook::callHook: no inputEncoder given");

  VorbisDecoder& decoder = static_cast<VorbisDecoder&>(*outputDecoder);
  VorbisEncoder& encoder = static_cast<VorbisEncoder&>(*inputEncoder);

  if (copy) {
    packet->setStreamNo(encoder.getStreamNo());
    packetList.push_back(packet);
  } else {
    // relevant packet
    try {
      decoder << packet;
      while ( decoder.isAvailable() ) {
        decoder >> audioPacket;

        if ( changeAudioSamplerate || changeChannel ) {
          AudioPacket tmp;

          if ( converter.resample ( audioPacket,tmp ) ) {
            encoder << tmp;

          }
        } else {
          encoder << audioPacket;
        }
        while ( encoder.isAvailable() ) {
          OggPacket pckt;
          encoder >> pckt;

          packetList.push_back(pckt);
        }
// 16868466
      }

    } catch ( std::exception error ) {
      logger.error() << "Exception: " << error.what();
    }
  }
}

void AudioHook::flush()
{
  if (!outputDecoder)
    throw OggException("AudioHook::callHook: no outputDecoder given");

  if (!inputEncoder)
    throw OggException("AudioHook::callHook: no inputEncoder given");

  VorbisDecoder& decoder = static_cast<VorbisDecoder&>(*outputDecoder);
  VorbisEncoder& encoder = static_cast<VorbisEncoder&>(*inputEncoder);

  /* write resampled data, if there is some */
  if (converter.resampleflush(audioPacket)) {

    if ( audioPacket->getLength() > 0 )
      encoder << audioPacket;
  }

  encoder.flush();

  while ( encoder.isAvailable() ) {

    OggPacket pckt;
    encoder >> pckt;

    packetList.push_back(pckt);

  }

}

OggType AudioHook::getType() const
{
  return(OggType::vorbis);
}

static bool operator==(const vorbis_info& info1, const vorbis_info& info2)
{
  return ((info1.bitrate_lower == info2.bitrate_lower) &&
          (info1.bitrate_nominal == info2.bitrate_nominal) &&
          (info1.bitrate_upper == info2.bitrate_upper) &&
          (info1.bitrate_window == info2.bitrate_window) &&
          (info1.channels == info2.channels) &&
          (info1.rate == info2.rate)
         );
}

static bool operator!=(const vorbis_info& info1, const vorbis_info& info2)
{
  return(!(info1==info2));
}
