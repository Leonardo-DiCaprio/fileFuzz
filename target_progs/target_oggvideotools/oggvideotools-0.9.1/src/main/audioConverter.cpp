#include "audioConverter.h"

#include <cmath>
#include <iostream>

#include "libresample/libresample.h"
#include "log.h"

AudioConverter::AudioConverter()
  : channelData(0), tmp(0), handle(0), used(0), ratio(0), channels(0)
{
}

AudioConverter::~AudioConverter()
{
}

void AudioConverter::initResample(uint8 _channels, double _ratio)
{

  ratio = _ratio;
  channels = _channels;

  handle = (void **) new char[channels * sizeof(void *)];

  channelData = new float*[channels];
  tmp = new float*[channels];

  for (uint8 c=0; c<channels; c++) {
    handle[c] = resample_open(1, ratio, ratio);
    channelData[c] = new float[maxSamples];
    tmp[c] = new float[maxSamples];
  }
}

void AudioConverter::closeResample()
{

  for (uint8 c=0; c<channels; c++) {
    delete[] channelData[c];
    delete[] tmp[c];

    resample_close(handle[c]);
  }

  delete[] channelData;
  delete[] tmp;
  delete[] handle;
}

bool AudioConverter::resample(AudioPacket packet, AudioPacket& resampled)
{

  uint32 length;

  // upmix -> we want stereo and have mono
  if ((channels == 2) && (packet->getChannels() == 1)) {

    for (uint32 pos(0); pos < packet->getLength(); ++pos)
      channelData[0][used+pos] = packet->getDataOfChannel(0)[pos];

    for (uint32 pos(0); pos < packet->getLength(); ++pos)
      channelData[1][used+pos] = packet->getDataOfChannel(0)[pos];

  }

  // downmix -> we want mono and have stereo
  if ((channels == 1) && (packet->getChannels() == 2)) {

    for (uint32 pos(0); pos < packet->getLength(); ++pos) {
      channelData[0][used+pos] = packet->getDataOfChannel(0)[pos]*0.5 +
                                 packet->getDataOfChannel(1)[pos]*0.5;

      if (channelData[0][used+pos]> 1.0)
        channelData[0][used+pos] = 1.0;
    }
  }

  // plane copy
  if (channels == packet->getChannels()) {

    for (uint32 c(0); c < channels; ++c)
      for (uint32 pos(0); pos < packet->getLength(); ++pos)
        channelData[c][used+pos] = packet->getDataOfChannel(c)[pos];

  }

  if (ratio == 1.0) {
    length = packet->getLength();
    AudioPacketInternal* newInternalPacket = new AudioPacketInternal(channels, length);

    for (uint32 i(0); i<channels; ++i) {
      newInternalPacket->setDataOfChannel(i,channelData[i]);
    }

    resampled = AudioPacket(newInternalPacket);
    return(true);
  }


  uint32 availableSamples = used+packet->getLength();
  int32 inUsed=0;

  // do resampling
  for (uint32 i(0); i<channels; ++i) {
    length = resample_process(handle[i], ratio, channelData[i], availableSamples, 0,
                              &inUsed, tmp[i], maxSamples);
  }

  AudioPacketInternal* newInternalPacket = new AudioPacketInternal(channels, length);

  for (uint32 i(0); i<channels; ++i) {
    newInternalPacket->setDataOfChannel(i,tmp[i]);
  }

  resampled = AudioPacket(newInternalPacket);

  // save data
  used = availableSamples-inUsed;
  for (uint32 i(0); i < used; ++i) {
    for (uint32 ch(0); ch<channels; ++ch) {
      channelData[ch][i] = channelData[ch][inUsed + i];
    }

    return (length> 0);
  }

  return(true);
}

bool AudioConverter::resampleflush(AudioPacket & resampled)
{

  if (used==0)
    return(false);

  logger.error() << "AudioConverter::resampleflush: not implemented "<<used<<" original samples are not resampled\n";

  return(false);

}

