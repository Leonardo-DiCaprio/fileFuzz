#ifndef AUDIOCONVERTER_H_
#define AUDIOCONVERTER_H_

#include "audioPacket.h"

class AudioConverter {

  static const uint32 maxSamples = 4096;

  float** channelData;
  float** tmp;

  void** handle;

  uint32 used;
  double ratio;
  uint8 channels;

public:

  AudioConverter();
  virtual ~AudioConverter();

  void initResample(uint8 channels, double ratio);

  bool resample(AudioPacket packet, AudioPacket& resampled);

  bool resampleflush(AudioPacket& resampled);

  void closeResample();
};

#endif /*AUDIOCONVERTER_H_*/
