#include "mediaInputEncoder.h"

MediaInputEncoder::MediaInputEncoder(const uint8 _streamNo)
  : streamNo(_streamNo)
{
}

MediaInputEncoder::~MediaInputEncoder()
{
}

uint32 MediaInputEncoder::getStreamNo() const
{
  return(streamNo);
}