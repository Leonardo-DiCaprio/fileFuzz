#include "audioPacket.h"

#include <cstring>
#include <iostream>
#include "log.h"

AudioPacketInternal::AudioPacketInternal() :
  pcmData(0), length(0), channels(0)
{
}

AudioPacketInternal::~AudioPacketInternal()
{
  cleanup();
}

AudioPacketInternal::AudioPacketInternal(const AudioPacketInternal& packet) :
  pcmData(0), length(packet.length), channels(packet.channels)
{
  /* create memory region */
  initMem(channels, length);

  /* copy data */
  for (uint8 i(0); i<channels; ++i)
    setDataOfChannel(i, packet.pcmData[i]);
}

AudioPacketInternal::AudioPacketInternal(float** _dataPtr, uint32 _length,
    uint8 _channels) :
  pcmData(0), length(_length), channels(_channels)
{
  /* create memory region */
  initMem(channels, length);

  /* copy data */
  for (uint8 i(0); i<channels; ++i)
    setDataOfChannel(i, _dataPtr[i]);

}

AudioPacketInternal::AudioPacketInternal(uint8 _channels, uint32 _length) :
  length(_length), channels(_channels)
{
  /* create memory region */
  initMem(channels, length);
}

void AudioPacketInternal::initMem(uint8 _channels, uint32 _length)
{
  pcmData = new float*[_channels];

  for (uint8 ch(0); ch<_channels; ++ch)
    pcmData[ch] = new float[_length];

}

float** AudioPacketInternal::getAllChannels() const
{
  return (pcmData);
}

uint32 AudioPacketInternal::getLength() const
{
  return (length);
}

uint8 AudioPacketInternal::getChannels() const
{
  return (channels);
}

float* AudioPacketInternal::getDataOfChannel(uint8 channel) const
{
  if (channel < channels)
    return (pcmData[channel]);

  return (0);
}

void AudioPacketInternal::setDataOfChannel(uint8 channel, float* _dataPtr)
{
  if ((pcmData == 0) || (channel >= channels) || (pcmData[channel] == 0))
    return;

  memcpy(pcmData[channel], _dataPtr, length*sizeof(float));

}

void AudioPacketInternal::cleanup()
{

  if (pcmData != 0) {
    for (uint8 ch(0); ch<channels; ++ch) {
      delete[] pcmData[ch];
    }
    delete[] pcmData;

  }

  channels = 0;
  length = 0;
}

