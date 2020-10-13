#ifndef audioPacket_h
#define audioPacket_h

#include "definition.h"
#include <memory>

class AudioPacketInternal {

protected:
  float** pcmData;
  uint32 length;
  uint8 channels;

  void initMem(uint8 channels, uint32 length);

public:

  AudioPacketInternal();
  AudioPacketInternal(const AudioPacketInternal& packet);
  AudioPacketInternal(uint8 channels, uint32 length);
  AudioPacketInternal(float** dataPtr, uint32 length, uint8 channels);
  ~AudioPacketInternal();

  uint32 getLength() const;
  uint8 getChannels() const;
  float** getAllChannels() const;

  float* getDataOfChannel(uint8 channel) const;
  void setDataOfChannel(uint8 channel, float* data);

  void cleanup();
};

typedef std::shared_ptr<AudioPacketInternal> AudioPacket;

#endif
